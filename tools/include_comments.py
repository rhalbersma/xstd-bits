#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Check that every name an include's trailing comment lists appears in its file's code.

    #include <ranges> // iota, subrange

says the file needs `iota` and `subrange` from <ranges>. misc-include-cleaner checks the include,
not the names after it, and runs on the public headers alone, so a name the code stopped using
stays in the comment until a reader trusts it. Each listed name must appear in the file outside
its include lines and comments:

- an identifier or a qualified name such as `ranges::equal`, as a whole word;
- a prefix such as `check_*`, as the start of a word.

An umbrella's `IWYU pragma: export` line lists what it publishes rather than what it uses, an
`IWYU pragma: keep` line what the include provides without the file spelling it, an `operator@`
is used without being spelled by name, and a comment that is prose rather than a list of names
says why the include is there; none of the four is checked.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TREES = ["include", "test", "benchmark", "examples"]
SOURCES = {".hpp", ".cpp", ".h"}

INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]\s*(?://\s*(.*))?$')
PRAGMAS = ("IWYU pragma: export", "IWYU pragma: keep")
NAME = re.compile(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*")
PREFIX = re.compile(r"([A-Za-z_]\w*)\*")
COMMENTS = re.compile(r"//[^\n]*|/\*.*?\*/", re.DOTALL)


def listed(comment):
    """The names a trailing comment lists, as patterns over the code, or none for prose and pragmas."""
    if any(pragma in comment for pragma in PRAGMAS):
        return []
    items = [item.strip() for item in comment.split(",")]
    patterns = []
    for item in items:
        if NAME.fullmatch(item):
            patterns.append((item, re.compile(rf"(?<!\w){re.escape(item)}(?!\w)")))
        elif match := PREFIX.fullmatch(item):
            patterns.append((item, re.compile(rf"(?<!\w){re.escape(match.group(1))}")))
        elif item.startswith("operator"):
            continue
        else:
            return []
    return patterns


def check(path, errors):
    """Append to errors each name an include comment in path lists that its code does not use."""
    lines = path.read_text(encoding="utf-8").splitlines()
    includes = [(lineno, match) for lineno, line in enumerate(lines, 1) if (match := INCLUDE.match(line))]
    code = COMMENTS.sub("", "\n".join(line for line in lines if not INCLUDE.match(line)))
    for lineno, match in includes:
        for name, pattern in listed(match.group(2) or ""):
            if not pattern.search(code):
                errors.append(f"{path.relative_to(ROOT)}:{lineno}: <{match.group(1)}> lists {name}, which the file does not use")


def main():
    errors = []
    count = 0
    for tree in TREES:
        for path in sorted((ROOT / tree).rglob("*")):
            if path.suffix in SOURCES and path.is_file():
                check(path, errors)
                count += 1
    print(f"{count} files, {len(errors)} unused names")
    for error in errors:
        print(f"error: {error}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
