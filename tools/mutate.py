#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Mutation testing for the headers under include/: does a test fail when the code breaks?

Each mutant changes one operator, literal or call at one site of one header. It is compiled
into a private copy of include/ with the build's own compile and link commands, so mutants
run side by side and the checkout is never touched. A mutant is run against the test targets
whose objects depend on its header, the header's own mirrored test first, and stops at the
first one that notices.

    killed      a test failed, timed out, or rejected it at compile time (a static_assert,
                or a constant expression that is no longer one)
    survived    every dependent test built and passed
    equivalent  it survived, and doc/mutation.md says why no test can tell it apart
    stillborn   it does not compile for a reason no test states, such as a type error

doc/mutation.md identifies a mutant by its header, the text of its line and its place in that
text, never by line number: an edit to the line brings the mutant back for review.

The build directory must be a Ninja build of this repository with BUILD_TESTING on and
CMAKE_EXPORT_COMPILE_COMMANDS on, built once before the first run.
"""

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from dataclasses import asdict, dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE = ROOT / "include"

# One alternative per match: (name, pattern over code with comments and literals blanked, replacement).
OPERATORS = [
        ("ror", r"(?<= )<(?= )", "<="),
        ("ror", r"(?<= )<=(?= )", "<"),
        ("ror", r"(?<= )>(?= )", ">="),
        ("ror", r"(?<= )>=(?= )", ">"),
        ("ror", r"(?<= )==(?= )", "!="),
        ("ror", r"(?<= )!=(?= )", "=="),
        ("aor", r"(?<= )\+(?= )", "-"),
        ("aor", r"(?<= )-(?= )", "+"),
        ("aor", r"(?<= )\+=(?= )", "-="),
        ("aor", r"(?<= )-=(?= )", "+="),
        ("aor", r"(?<= )\*(?= )", "+"),
        ("aor", r"(?<= )/(?= )", "*"),
        ("aor", r"(?<= )%(?= )", "/"),
        ("aor", r"\+\+", "--"),
        ("aor", r"--(?![=>])", "++"),
        ("bor", r"(?<= )&(?= )", "|"),
        ("bor", r"(?<= )\|(?= )", "&"),
        ("bor", r"(?<= )\^(?= )", "|"),
        ("bor", r"(?<= )&=(?= )", "|="),
        ("bor", r"(?<= )\|=(?= )", "&="),
        ("bor", r"(?<= )\^=(?= )", "|="),
        ("bor", r"~(?=[\w(])", ""),
        ("sor", r"(?<= )<<(?= )", ">>"),
        ("sor", r"(?<= )>>(?= )", "<<"),
        ("sor", r"\bshl\(", "shr("),
        ("sor", r"\bshr\(", "shl("),
        ("lcr", r"(?<= )and(?= )", "or"),
        ("lcr", r"(?<= )or(?= )", "and"),
        ("lcr", r"\bnot ", ""),
        ("lit", r"\b0UZ\b", "1UZ"),
        ("lit", r"\b1UZ\b", "0UZ"),
        ("lit", r"\b0U\b", "1U"),
        ("lit", r"\b1U\b", "0U"),
        ("lit", r"(?<=\w)\{1\}", "{0}"),
        ("lit", r"\btrue\b", "false"),
        ("lit", r"\bfalse\b", "true"),
        ("call", r"\bcountr_zero\b", "countl_zero"),
        ("call", r"\bcountl_zero\b", "countr_zero"),
        ("call", r"\bcountr_one\b", "countl_one"),
        ("call", r"\bcountl_one\b", "countr_one"),
        ("call", r"\bmin\(", "max("),
        ("call", r"\bmax\(", "min("),
]

# Lines that declare rather than compute: a mutant there changes what overload is chosen, not what it does.
DECLARATIVE = re.compile(r"^\s*(#|template\s*<|concept\b|requires\b|using\b|static_assert\b|friend\s+(class|struct)\b|namespace\b|->|assert\()")

# A diagnostic that a test wrote: the mutant was caught at compile time rather than stillborn.
COMPILE_TIME_CHECK = re.compile(r"static assertion failed|static assertion expression is not an integral constant expression|static_assert failed|not a constant expression|must be initialized by a constant expression|constexpr variable .* must be initialized")


@dataclass
class Mutant:
        header: str
        line: int
        column: int
        operator: str
        original: str
        replacement: str
        source_line: str
        status: str = "pending"
        target: str = ""
        detail: str = ""
        seconds: float = 0.0

        @property
        def key(self):
                """What identifies this mutant across edits elsewhere in its header."""
                return (self.header, self.source_line, self.offset, self.original, self.replacement)

        @property
        def offset(self):
                """The column within source_line, which drops the line's indentation."""
                raw = (INCLUDE / self.header).read_text().split("\n")[self.line - 1]
                return self.column - (len(raw) - len(raw.lstrip()))


@dataclass
class Target:
        name: str
        source: str
        compile: list[str]
        object: str
        link: list[str]
        executable: str
        baseline: float = 0.0
        headers: set[str] = field(default_factory=set)


def blank(text):
        """text with comments, string literals and character literals replaced by spaces, offsets kept."""
        out = list(text)
        i, n = 0, len(text)
        while i < n:
                if text.startswith("//", i):
                        j = text.find("\n", i)
                        j = n if j < 0 else j
                elif text.startswith("/*", i):
                        j = text.find("*/", i + 2)
                        j = n if j < 0 else j + 2
                elif text[i] in "\"'" and not (text[i] == "'" and i > 0 and text[i - 1].isalnum()):
                        quote, j = text[i], i + 1
                        while j < n and text[j] != quote:
                                j += 2 if text[j] == "\\" else 1
                        j += 1
                else:
                        i += 1
                        continue
                for k in range(i, min(j, n)):
                        if out[k] != "\n":
                                out[k] = " "
                i = j
        return "".join(out)


UNEVALUATED = re.compile(r"\b(noexcept|decltype|sizeof|alignof)\s*\(")


def blank_unevaluated(code):
        """code with the operands of noexcept, decltype, sizeof and alignof blanked: they state a type, not a computation."""
        out = list(code)
        for match in UNEVALUATED.finditer(code):
                depth, i = 1, match.end()
                while i < len(code) and depth:
                        depth += {"(": 1, ")": -1}.get(code[i], 0)
                        if depth and code[i] != "\n":
                                out[i] = " "
                        i += 1
        return "".join(out)


def sites(header, include_asserts=False):
        """Every mutant of header, in the order its sites appear."""
        text = (INCLUDE / header).read_text()
        code = blank_unevaluated(blank(text))
        lines = text.splitlines()
        starts = [0]
        for line in lines:
                starts.append(starts[-1] + len(line) + 1)
        result = []
        for number, (begin, end) in enumerate(zip(starts, starts[1:]), start=1):
                stripped = code[begin:end]
                declarative = DECLARATIVE.match(stripped)
                if declarative and not (include_asserts and declarative.group(1) == "assert("):
                        continue
                for name, pattern, replacement in OPERATORS:
                        for match in re.finditer(pattern, stripped):
                                result.append(Mutant(header, number, match.start() + 1, name, match.group(0), replacement, lines[number - 1].strip()))
        return sorted(result, key=lambda m: (m.line, m.column))


def apply(mutant):
        """The text of mutant's header with the mutation made."""
        lines = (INCLUDE / mutant.header).read_text().split("\n")
        line = lines[mutant.line - 1]
        at = mutant.column - 1
        assert line[at:at + len(mutant.original)] == mutant.original
        lines[mutant.line - 1] = line[:at] + mutant.replacement + line[at + len(mutant.original):]
        return "\n".join(lines)


def run(command, cwd, timeout):
        """(returncode, combined output), with returncode None on a timeout."""
        try:
                done = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=timeout)
                return done.returncode, done.stdout
        except subprocess.TimeoutExpired as expired:
                return None, (expired.stdout or b"").decode(errors="replace") if isinstance(expired.stdout, bytes) else (expired.stdout or "")


PIPE = re.compile(r"(?<!\\)\|")
EQUIVALENTS = ROOT / "doc" / "mutation.md"
EQUIVALENT_COLUMNS = ["header", "line", "column", "mutation", "reason"]
MUTATION = re.compile(r"^`(.*)` → `(.*)`$")


def cells(line):
        """The cells of a markdown table row, split on unescaped pipes, each unescaped."""
        return [cell.strip().replace("\\|", "|") for cell in PIPE.split(line.strip())[1:-1]]


def code(cell):
        """The text inside a cell's one pair of backticks."""
        return cell[1:-1] if cell.startswith("`") and cell.endswith("`") else cell


def read_equivalents(path):
        """{key: reason} from the one table in path, whose columns are EQUIVALENT_COLUMNS."""
        if not path.exists():
                return {}
        result, header = {}, None
        for number, line in enumerate(path.read_text().splitlines(), start=1):
                if not line.startswith("|"):
                        continue
                row = cells(line)
                if header is None:
                        header = row
                        if header != EQUIVALENT_COLUMNS:
                                raise SystemExit(f"{path}:{number}: columns are {header}, expected {EQUIVALENT_COLUMNS}")
                        continue
                if all(set(cell) <= set(":-") for cell in row):
                        continue
                match = MUTATION.match(row[3])
                if len(row) != len(EQUIVALENT_COLUMNS) or not match or not row[4]:
                        raise SystemExit(f"{path}:{number}: expected `header` | `line` | column | `from` → `to` | reason")
                result[(code(row[0]), code(row[1]), int(row[2]), match.group(1), match.group(2))] = row[4]
        return result


def without_depfile(arguments):
        """arguments less the compiler's dependency-file flags, which would write into the build's own tree."""
        result, skip = [], False
        for argument in arguments:
                if skip:
                        skip = False
                elif argument in ("-MF", "-MT", "-MQ"):
                        skip = True
                elif argument not in ("-MD", "-MMD"):
                        result.append(argument)
        return result


def load_targets(build):
        """The test executables of build, each with its one compile command, link command and header dependencies."""
        database = {Path(entry["file"]).resolve().as_posix(): entry for entry in json.loads((build / "compile_commands.json").read_text())}
        deps = subprocess.run(["ninja", "-C", str(build), "-t", "deps"], stdout=subprocess.PIPE, text=True, check=True).stdout
        headers_of, current = {}, None
        for line in deps.splitlines():
                if line and not line.startswith(" "):
                        current = line.split(":")[0]
                        headers_of[current] = set()
                elif line.strip() and current is not None:
                        path = Path(line.strip())
                        path = (build / path).resolve() if not path.is_absolute() else path.resolve()
                        if path.is_relative_to(INCLUDE):
                                headers_of[current].add(path.relative_to(INCLUDE).as_posix())
        targets = {}
        for entry in database.values():
                command = entry.get("command")
                arguments = entry.get("arguments") or shlex.split(command)
                obj = arguments[arguments.index("-o") + 1]
                match = re.search(r"CMakeFiles/(test\.[^/]+)\.dir/", obj)
                if not match:
                        continue
                name = match.group(1)
                commands = subprocess.run(["ninja", "-C", str(build), "-t", "commands", name], stdout=subprocess.PIPE, text=True, check=True).stdout.splitlines()
                link = shlex.split(next(part for part in commands[-1].split("&&") if " -o " in part))
                # The linker's dependency file is the build's own; a worker that wrote it would race the others.
                for at in reversed(range(len(link) - 1)):
                        if link[at] == "-Xlinker" and link[at + 1].startswith("--dependency-file="):
                                del link[at:at + 2]
                executable = link[link.index("-o") + 1]
                targets[name] = Target(name, entry["file"], arguments, obj, link, executable, headers=headers_of.get(obj, set()))
        return targets


def mirror(header):
        """The test target that mirrors header: xstd/bits/bit_algorithm/bit_copy.hpp is test.bits.bit_algorithm.bit_copy."""
        stem = re.sub(r"^xstd/", "", header)
        stem = re.sub(r"[.]hpp$", "", stem)
        return "test." + stem.replace("/", ".")


def order(header, targets, only):
        """The targets depending on header, the mirrored test first, then the ones that include the fewest headers."""
        dependents = [t for t in targets.values() if header in t.headers and (only is None or re.search(only, t.name))]
        own = mirror(header)
        return sorted(dependents, key=lambda t: (t.name != own, len(t.headers), t.name))


class Worker:
        """A private copy of include/ and a scratch directory for objects and executables."""

        def __init__(self, scratch, build):
                self.root = Path(tempfile.mkdtemp(prefix="worker-", dir=scratch))
                self.include = self.root / "include"
                shutil.copytree(INCLUDE, self.include)
                self.build = build

        def rewrite(self, arguments, mapping):
                result = []
                for argument in arguments:
                        for old, new in mapping.items():
                                argument = argument.replace(old, new)
                        result.append(argument)
                return result

        def attempt(self, mutant, target, timeout, link_timeout):
                """'killed', 'survived' or 'stillborn' for mutant against one target, with a line of evidence."""
                obj = (self.root / Path(target.object).name).as_posix()
                exe = (self.root / Path(target.executable).name).as_posix()
                compile_command = self.rewrite(target.compile, {INCLUDE.as_posix(): self.include.as_posix()})
                at = compile_command.index("-o")
                compile_command[at + 1] = obj
                compile_command = without_depfile(compile_command)
                returncode, output = run(compile_command, self.build, link_timeout)
                if returncode is None:
                        return "stillborn", "compile timed out"
                if returncode != 0:
                        first = next((line for line in output.splitlines() if "error" in line), output.strip()[:200])
                        return ("killed" if COMPILE_TIME_CHECK.search(output) else "stillborn"), first.strip()[:300]
                link_command = self.rewrite(target.link, {target.object: obj})
                link_command[link_command.index("-o") + 1] = exe
                returncode, output = run(link_command, self.build, link_timeout)
                if returncode != 0:
                        return "stillborn", "link failed"
                returncode, output = run([exe, "--log_level=nothing", "--report_level=no"], self.build, timeout(target))
                if returncode is None:
                        return "killed", "timed out"
                if returncode != 0:
                        return "killed", f"exit {returncode}"
                return "survived", ""

        def evaluate(self, mutant, chain, timeout, link_timeout):
                original = (self.include / mutant.header).read_text()
                (self.include / mutant.header).write_text(apply(mutant))
                start = time.monotonic()
                try:
                        mutant.status = "survived"
                        for target in chain:
                                status, detail = self.attempt(mutant, target, timeout, link_timeout)
                                if status != "survived":
                                        mutant.status, mutant.target, mutant.detail = status, target.name, detail
                                        break
                finally:
                        (self.include / mutant.header).write_text(original)
                        mutant.seconds = round(time.monotonic() - start, 1)
                return mutant


def main():
        parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
        parser.add_argument("headers", nargs="+", help="headers relative to include/, e.g. xstd/bits/bit_algorithm/bit_copy.hpp")
        parser.add_argument("--build", type=Path, required=True, help="the configured and built Ninja build directory")
        parser.add_argument("--jobs", type=int, default=os.cpu_count(), help="mutants compiled and run side by side")
        parser.add_argument("--tests", help="only the test targets whose name matches this regex")
        parser.add_argument("--max-targets", type=int, default=0, help="stop widening after this many targets (0: every dependent)")
        parser.add_argument("--lines", help="only sites on these lines, as 10-20,35")
        parser.add_argument("--operators", help="only these operator classes, comma-separated: " + ", ".join(sorted({o[0] for o in OPERATORS})))
        parser.add_argument("--include-asserts", action="store_true", help="also mutate assert() preconditions")
        parser.add_argument("--list", action="store_true", help="list the mutants and exit")
        parser.add_argument("--output", type=Path, help="write the results as JSON here")
        parser.add_argument("--equivalents", type=Path, default=EQUIVALENTS, help="the classified survivors, by default doc/mutation.md")
        args = parser.parse_args()

        mutants = [m for header in args.headers for m in sites(header, args.include_asserts)]
        filtered = bool(args.operators or args.lines or args.include_asserts)
        if args.operators:
                wanted = set(args.operators.split(","))
                mutants = [m for m in mutants if m.operator in wanted]
        if args.lines:
                keep = set()
                for part in args.lines.split(","):
                        first, _, last = part.partition("-")
                        keep.update(range(int(first), int(last or first) + 1))
                mutants = [m for m in mutants if m.line in keep]
        if args.list:
                for m in mutants:
                        print(f"{m.header}:{m.line}:{m.column}: {m.operator} {m.original!r} -> {m.replacement!r}    {m.source_line}")
                print(f"{len(mutants)} mutants", file=sys.stderr)
                return 0

        build = args.build.resolve()
        targets = load_targets(build)
        chains = {}
        for header in args.headers:
                chain = order(header, targets, args.tests)
                if args.max_targets:
                        chain = chain[:args.max_targets]
                if not chain:
                        print(f"error: no test target depends on {header}", file=sys.stderr)
                        return 2
                chains[header] = chain
                print(f"{header}: {len(chain)} dependent targets, first {chain[0].name}", file=sys.stderr)

        for chain in chains.values():
                for target in chain:
                        if target.baseline == 0.0:
                                start = time.monotonic()
                                returncode, output = run([target.executable, "--log_level=nothing", "--report_level=no"], build, None)
                                target.baseline = time.monotonic() - start
                                if returncode != 0:
                                        print(f"error: {target.name} fails unmutated:\n{output[-2000:]}", file=sys.stderr)
                                        return 2

        def timeout(target):
                return max(30.0, 10.0 * target.baseline)

        scratch = Path(tempfile.mkdtemp(prefix="xstd-bits-mutate-"))
        workers = [Worker(scratch, build) for _ in range(args.jobs)]
        free = list(workers)

        def task(mutant):
                worker = free.pop()
                try:
                        return worker.evaluate(mutant, chains[mutant.header], timeout, 1800)
                finally:
                        free.append(worker)

        counts = {"killed": 0, "survived": 0, "stillborn": 0}
        try:
                with ThreadPoolExecutor(max_workers=args.jobs) as pool:
                        for done, mutant in enumerate(pool.map(task, mutants), start=1):
                                counts[mutant.status] += 1
                                mark = {"killed": ".", "survived": "S", "stillborn": "x"}[mutant.status]
                                where = f" [{mutant.target}: {mutant.detail}]" if mutant.status != "survived" and mutant.detail else ""
                                print(f"{mark} {done}/{len(mutants)} {mutant.header}:{mutant.line}:{mutant.column} {mutant.original!r}->{mutant.replacement!r} {mutant.seconds}s{where}", flush=True)
                                if args.output:
                                        args.output.write_text(json.dumps([asdict(m) for m in mutants], indent=1))
        finally:
                shutil.rmtree(scratch, ignore_errors=True)

        equivalents = read_equivalents(args.equivalents)
        stale = []
        for m in mutants:
                if m.key in equivalents:
                        if m.status == "survived":
                                m.status, m.detail = "equivalent", equivalents[m.key]
                                counts["survived"] -= 1
                                counts["equivalent"] = counts.get("equivalent", 0) + 1
                        else:
                                stale.append(f"listed as equivalent but {m.status}: {m.header}:{m.line}:{m.column}: {m.original!r} -> {m.replacement!r}")
        if not filtered:
                generated = {m.key for m in mutants}
                stale += [f"listed as equivalent but no longer generated: {k[0]}: `{k[1]}` column {k[2]}: {k[3]!r} -> {k[4]!r}" for k in equivalents if k[0] in args.headers and k not in generated]
        if args.output:
                args.output.write_text(json.dumps([asdict(m) | {"offset": m.offset} for m in mutants], indent=1))

        valid = counts["killed"] + counts["survived"] + counts.get("equivalent", 0)
        score = 100.0 * (counts["killed"] + counts.get("equivalent", 0)) / valid if valid else 100.0
        print(f"killed {counts['killed']}, equivalent {counts.get('equivalent', 0)}, survived {counts['survived']}, stillborn {counts['stillborn']}: score {score:.1f}%")
        for m in mutants:
                if m.status == "survived":
                        print(f"SURVIVED {m.header}:{m.line}:{m.column}: {m.original!r} -> {m.replacement!r}    {m.source_line}")
                        print(f"  | `{m.header}` | `{m.source_line.replace('|', chr(92) + '|')}` | {m.offset} | `{m.original.replace('|', chr(92) + '|')}` → `{m.replacement.replace('|', chr(92) + '|')}` | |")
        for line in stale:
                print(f"STALE {line}")
        return 1 if counts["survived"] or stale else 0


if __name__ == "__main__":
        sys.exit(main())
