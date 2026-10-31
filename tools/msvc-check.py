#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Compile test sources with MSVC on Compiler Explorer, as the MSVC ladder would, before a push.

tools/ci-local.sh runs every Linux leg, but no MSVC, so a warning only MSVC gives, such as C4127 on
an if whose condition a stateless lambda makes constant, waited for CI to find it. This script asks
Compiler Explorer's MSVC instead:

    tools/msvc-check.py test/src/bits/bit_algorithm/bit_find_if.cpp
    tools/msvc-check.py --changed            # the tests a branch touches, and the ones mirroring its headers

Each source is compiled, not linked, with the ladder's flags: /std:c++latest /W4 /WX /permissive- and
the test suite's definitions. Compiler Explorer sees one file, so the source is sent with every header
it reaches from this repository, xstd-ints, xstd-misc and the test suite's dependencies inlined, each
behind a #line directive, so a diagnostic names the real file and line. The include directories and
definitions come from a GCC 15 configure in build/msvc-check, which fetches the pinned dependencies as
every leg does. Boost picks its compiler configuration through macros Compiler Explorer cannot resolve
in one file, so the MSVC ones are inlined up front.

By default it asks both rungs the ladder builds on a pull request, VS 2022 and the newest MSVC; MinGW,
clang-cl, Apple Clang and Emscripten stay CI's. Compiler Explorer is a public service: what is sent is
this repository's own open source, and nothing else.
"""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CMAKE = Path.home() / ".local" / "bin" / "cmake"
API = "https://godbolt.org/api/compiler/{}/compile"

# The two rungs the MSVC ladder builds on a pull request: VS 2022, and VS 2026's Preview, which tracks the newest.
COMPILERS = ["vcpp_v19_44_VS17_14_x64", "vcpp_v19_latest_x64"]

# The ladder's Debug build: CMake's MSVC defaults, the test suite's warnings and /WX, and its MSVC-only definition.
FLAGS = "/std:c++latest /W4 /WX /permissive- /bigobj /EHsc /Zc:inline /Od /c /D_SCL_SECURE_NO_WARNINGS"

# The first path components of the headers inlined; the standard library, and the C headers beside these, stay #include.
LIBRARIES = {"xstd", "test", "boost", "absl", "range", "meta", "concepts"}

# Where setup-toolchain.sh installs Boost, and where apt installs range-v3.
SYSTEM_ROOTS = [Path("/usr/local/include"), Path("/usr/include")]

# Boost's own configuration chooses these headers by macro; Compiler Explorer's MSVC has no Boost to choose from.
PRELUDE = """\
#define BOOST_NO_USER_CONFIG
#define BOOST_COMPILER_CONFIG <cstddef>
#define BOOST_STDLIB_CONFIG <cstddef>
#define BOOST_PLATFORM_CONFIG <cstddef>
#include <boost/config/compiler/visualc.hpp>
#include <cstddef>
#include <boost/config/stdlib/dinkumware.hpp>
#include <boost/config/platform/win32.hpp>
"""

INCLUDE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')
HAS_INCLUDE = re.compile(r'__has_include\s*\(\s*([<"])([^>"]+)[>"]\s*\)')
OPEN = re.compile(r"^\s*#\s*if")
CLOSE = re.compile(r"^\s*#\s*endif")
GUARD = re.compile(r"^\s*#\s*(ifndef\s+\w+|if\s+!\s*defined)")


class Amalgamation:
    """One source with every library header it reaches inlined, each the first time it is reached outside a condition."""

    def __init__(self, roots):
        self.roots = roots
        self.done = set()
        self.stack = []

    def find(self, name, quote, here):
        if name.split("/")[0] not in LIBRARIES and quote == "<":
            return None
        candidates = ([here.parent / name] if quote == '"' else []) + [root / name for root in self.roots]
        for path in candidates:
            if path.is_file() and (quote == '"' or name.split("/")[0] in LIBRARIES):
                return path.resolve()
        return None

    def emit(self, path, out):
        self.stack.append(path)
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        guarded = any(GUARD.match(line) for line in lines[:40])
        depth = 0
        out.append(f'#line 1 "{path}"')
        for number, line in enumerate(lines, 1):
            if CLOSE.match(line):
                depth -= 1
            elif OPEN.match(line):
                depth += 1
            if re.match(r"^\s*#\s*pragma\s+once", line):
                out.append("")
                continue
            line = HAS_INCLUDE.sub(lambda m: "1" if self.find(m.group(2), m.group(1), path) else m.group(0), line)
            match = INCLUDE.match(line)
            target = self.find(match.group(2), match.group(1), path) if match else None
            if target is None:
                out.append(line)
                continue
            conditional = depth > (1 if guarded else 0)
            if target not in self.done and target not in self.stack:
                if not conditional:
                    self.done.add(target)
                self.emit(target, out)
                out.append(f'#line {number + 1} "{path}"')
            else:
                out.append("")
        self.stack.pop()


def configure(build):
    """The compile commands of a GCC 15 configure, which fetches the dependencies the tests include."""
    commands = build / "compile_commands.json"
    if not commands.exists():
        cmake = str(CMAKE) if CMAKE.exists() else "cmake"
        subprocess.run([cmake, "-S", str(ROOT), "-B", str(build), "-G", "Ninja", "-DCMAKE_CXX_COMPILER=g++-15", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], check=True, stdout=subprocess.DEVNULL)
    return {Path(entry["file"]).resolve(): entry for entry in json.loads(commands.read_text())}


def roots_and_definitions(entry):
    """The include directories and definitions of one compile command, in the order the compiler searches them."""
    words = shlex.split(entry["command"]) if "command" in entry else entry["arguments"]
    roots, definitions = [], []
    for i, word in enumerate(words):
        for flag in ("-I", "-isystem"):
            if word == flag:
                roots.append(Path(words[i + 1]))
            elif word.startswith(flag) and len(word) > len(flag):
                roots.append(Path(word[len(flag):]))
        if word.startswith("-D"):
            definitions.append(word[2:])
    return roots + SYSTEM_ROOTS, definitions


def changed_sources(base):
    """The test sources a branch changes, and those whose path mirrors a header it changes."""
    names = subprocess.run(["git", "-C", str(ROOT), "diff", "--name-only", f"{base}...HEAD"], check=True, capture_output=True, text=True).stdout.split()
    sources = set()
    for name in names:
        if name.startswith("test/src/") and name.endswith(".cpp"):
            sources.add(ROOT / name)
        elif name.startswith("include/xstd/bits/") and name.endswith(".hpp"):
            mirror = ROOT / "test" / "src" / "bits" / name.removeprefix("include/xstd/bits/").replace("/detail/", "/")
            if mirror.with_suffix(".cpp").exists():
                sources.add(mirror.with_suffix(".cpp"))
    return sorted(source for source in sources if source.exists())


def compile_on(compiler, source, flags):
    body = {"source": source, "options": {"userArguments": flags, "filters": {"execute": False}}}
    request = urllib.request.Request(API.format(compiler), data=json.dumps(body).encode(), headers={"Content-Type": "application/json", "Accept": "application/json"})
    with urllib.request.urlopen(request, timeout=600) as response:
        result = json.load(response)
    diagnostics = [line.get("text", "") for line in result.get("stderr", []) + result.get("stdout", [])]
    return result.get("code"), [text for text in diagnostics if "warning" in text or "error" in text]


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("sources", nargs="*", type=Path, help="test sources to compile")
    parser.add_argument("--changed", nargs="?", const="origin/main", metavar="BASE", help="also every test the branch's diff from BASE (origin/main) reaches")
    parser.add_argument("--compiler", action="append", help=f"a Compiler Explorer id, repeatable (default: {', '.join(COMPILERS)})")
    parser.add_argument("--build", type=Path, default=ROOT / "build" / "msvc-check", help="where to configure (default: build/msvc-check)")
    args = parser.parse_args()

    sources = [source.resolve() for source in args.sources] + (changed_sources(args.changed) if args.changed else [])
    if not sources:
        parser.error("no sources: name some, or pass --changed")
    commands = configure(args.build)
    args.build.mkdir(parents=True, exist_ok=True)
    (args.build / "msvc_prelude.hpp").write_text(PRELUDE)

    failures = 0
    for source in dict.fromkeys(sources):
        if source not in commands:
            print(f"{source}: no compile command; is it a test of the suite?", file=sys.stderr)
            failures += 1
            continue
        roots, definitions = roots_and_definitions(commands[source])
        out, amalgamation = [], Amalgamation(roots)
        amalgamation.emit(args.build / "msvc_prelude.hpp", out)
        amalgamation.emit(source, out)
        flags = " ".join([FLAGS] + [f"/D{definition}" for definition in definitions])
        relative = source.relative_to(ROOT)
        (args.build / relative.with_suffix(".msvc.cpp").name).write_text("\n".join(out))
        for compiler in args.compiler or COMPILERS:
            code, diagnostics = compile_on(compiler, "\n".join(out), flags)
            print(f"{'pass' if code == 0 else 'FAIL'} {compiler:28} {relative}")
            for text in diagnostics:
                print(f"    {text}")
            failures += code != 0
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
