#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Hold doc/vendor.md to the standard library test suites the build ran.

rhalbersma/cpp-ci's std_test_suite adapter writes a manifest.json for each suite: the
repository and tag it fetched, and for every file under a directory the mapping takes, its
SHA-256 and what became of it. This script renders a suite's manifest as its section of
doc/vendor.md, one row per test the mapping selects, and either writes that section or
checks that the table already holds it:

    tools/vendor.py write build/.../std-test-suite/libstdcxx/manifest.json
    tools/vendor.py check build/.../std-test-suite/libstdcxx/manifest.json

A check fails on a test the table lacks, on a row whose file no longer hashes to what the row
records, on a row naming a file the tree no longer has, and on an outcome that changed. An
upstream edit to a test, or a mapping entry added or removed, therefore shows up as a diff to
review rather than passing unseen. Each suite job checks its own suite, since no one build has
all three standard libraries.
"""

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TABLE = ROOT / "doc" / "vendor.md"

NAMES = {"libcxx": "libc++", "libstdcxx": "libstdc++", "msvc-stl": "MSVC STL"}

HEADER = """# Standard library test suites

Each standard library's own tests of the containers this library generalizes, and what became of each one when it ran
against them. `tools/vendor.py` writes a suite's section from the manifest that rhalbersma/cpp-ci's adapter leaves in
the build, and each suite job's `std-test-suite.<suite>.table` test fails when the section no longer matches it.

- **Mappings:** `test/std-test-suite/<suite>.toml` name the directories taken, the rewrites onto this library's types,
  and each departure.
- **Hashes:** the first twelve hexadecimal digits of the test file's SHA-256 at the pinned tag.

A row's `outcome` is one of:

| outcome | meaning |
| :--- | :--- |
| `runs` | built and run as written, after the rewrite |
| `compiles` | a test of declarations alone, built as written |
| `runs, adapted` | built and run with the lines the note lists changed or dropped, each for the departure it names |
| `compiles, adapted` | as `compiles`, with lines changed as for `runs, adapted` |
| `fails to compile` | expected not to compile, for the paragraph or reason the note gives |
| `fails at run time` | expected to fail when run, for the paragraph or reason the note gives |
| `not run` | left out, for the reason the note gives |
"""

SECTION = re.compile(r"^## .*?(?=^## |\Z)", re.MULTILINE | re.DOTALL)
UNSELECTED = re.compile(r"^does not match '(.*)'$")


def cell(text: str) -> str:
    return text.replace("|", "\\|").replace("\n", " ")


def outcome(test: dict) -> tuple[str, str]:
    """The row's outcome and note for one manifest entry."""
    if test["kind"] == "skip":
        return "not run", test["reason"]
    if "expect" in test:
        stage = {"compile": "fails to compile", "run": "fails at run time"}[test["expect"]]
        return stage, test["paragraph"]
    kind = {"run": "runs", "compile": "compiles"}[test["kind"]]
    if test.get("exceptions"):
        return f"{kind}, adapted", "; ".join(test["exceptions"])
    return kind, ""


def section(manifest: dict) -> str:
    """A suite's section: its pin, a row per selected test, and a count of the files the selection leaves out."""
    suite = manifest["suite"]
    lines = [
        f"## {NAMES.get(suite, suite)}",
        "",
        f"- **Pinned:** [{manifest['repository']}]({manifest['repository']}) at `{manifest['tag']}`.",
        f"- **Mapping:** `test/std-test-suite/{suite}.toml`.",
    ]
    unselected: dict[str, int] = {}
    rows = []
    for test in sorted(manifest["tests"], key=lambda t: t["path"]):
        match = UNSELECTED.match(test.get("reason", "")) if test["kind"] == "skip" else None
        if match:
            unselected[match.group(1)] = unselected.get(match.group(1), 0) + 1
            continue
        result, note = outcome(test)
        rows.append(f"| `{cell(test['path'])}` | `{test['sha256'][:12]}` | {result} | {cell(note)} |")
    for pattern, count in sorted(unselected.items()):
        lines.append(f"- **Not selected:** {count} files that do not match `{cell(pattern)}`, left to the standard library.")
    lines += ["", "| test | sha256 | outcome | note |", "| :--- | :--- | :--- | :--- |", *rows]
    return "\n".join(lines) + "\n"


def sections(text: str) -> dict[str, str]:
    """Each suite's section by its heading, as section() renders it: no trailing blank line."""
    return {m.group(0).splitlines()[0]: m.group(0).rstrip("\n") + "\n" for m in SECTION.finditer(text)}


def order(title: str) -> int:
    """The suites in NAMES' order, libc++ first; any other after them."""
    names = [f"## {name}" for name in NAMES.values()]
    return names.index(title) if title in names else len(names)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("action", choices=["write", "check"])
    parser.add_argument("manifests", nargs="+", type=Path, help="manifest.json files the adapter wrote")
    parser.add_argument("--table", type=Path, default=TABLE, help="the table, by default doc/vendor.md")
    args = parser.parse_args()

    text = args.table.read_text() if args.table.exists() else HEADER
    current = sections(text)
    status = 0
    for path in args.manifests:
        rendered = section(json.loads(path.read_text()))
        title = rendered.splitlines()[0]
        if args.action == "write":
            current[title] = rendered
        elif current.get(title) != rendered:
            print(f"{args.table}: '{title}' does not match {path}; run tools/vendor.py write {path}", file=sys.stderr)
            status = 1
    if args.action == "write":
        args.table.write_text(HEADER + "\n" + "\n".join(current[title] for title in sorted(current, key=order)))
    return status


if __name__ == "__main__":
    sys.exit(main())
