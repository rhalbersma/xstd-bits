#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Check that every `ranges::` in the code says whose ranges it means.

Three namespaces are called `ranges` here: `std::ranges`, range-v3's `::ranges`, and `xstd::ranges`,
which holds this library's algorithms and iterator operations. Inside `namespace xstd`, an
unqualified `ranges::copy` finds `xstd::ranges` before either of the others, so a call meant for
the standard library compiles against this one, or fails to compile far from the mistake. Each
`ranges::` in the code must therefore be written `std::ranges::`, `xstd::ranges::` or `::ranges::`.
Comments are not code, so an include comment naming `ranges::equal` is not checked.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TREES = ["include", "test", "benchmark", "fuzz", "examples"]
SOURCES = {".hpp", ".cpp", ".h"}

COMMENTS = re.compile(r"//[^\n]*|/\*.*?\*/", re.DOTALL)
RANGES = re.compile(r"(?<!\w)ranges::")
QUALIFIER = re.compile(r"(\w*)::$")
ALLOWED = {"std", "xstd", ""}


def blank_comments(text):
    """The text with each comment replaced by as many newlines as it spans, so line numbers stay."""
    return COMMENTS.sub(lambda match: "\n" * match.group(0).count("\n"), text)


def check(path, errors):
    """Append to errors each `ranges::` in path's code that names no namespace before it."""
    code = blank_comments(path.read_text(encoding="utf-8"))
    for match in RANGES.finditer(code):
        qualifier = QUALIFIER.search(code, 0, match.start())
        if qualifier is None or qualifier.group(1) not in ALLOWED:
            lineno = code.count("\n", 0, match.start()) + 1
            errors.append(f"{path.relative_to(ROOT)}:{lineno}: ranges:: without std::, xstd:: or a leading ::")


def main():
    errors = []
    count = 0
    for tree in TREES:
        for path in sorted((ROOT / tree).rglob("*")):
            if path.suffix in SOURCES and path.is_file():
                check(path, errors)
                count += 1
    print(f"{count} files, {len(errors)} unqualified ranges::")
    for error in errors:
        print(f"error: {error}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
