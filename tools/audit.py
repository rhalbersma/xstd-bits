#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Check the paragraph inventories under doc/audit/ against the citations in test/.

Each doc/audit/*.md holds one table with a row per numbered paragraph of a specification:

    | clause | ¶ | declaration | element | outcome | note |

A citation is `[stable.name]/13`, `[stable.name]/13,14` or `[stable.name]/9-16` anywhere in a
source under test/. The inventory is the denominator and the citations are the numerator.
"""

import argparse
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COLUMNS = ["clause", "¶", "declaration", "element", "outcome", "note"]
OUTCOMES = ["answered", "declined", "forced", "no-requirement", "gap"]
NEEDS_NOTE = {"declined", "forced", "no-requirement"}
SOURCES = {".cpp", ".hpp"}

CITATION = re.compile(r"\[([a-z][a-z0-9_.]*)\]/(\d+(?:-\d+)?(?:,\d+(?:-\d+)?)*)")
PIPE = re.compile(r"(?<!\\)\|")
RULE = re.compile(r"^:?-+:?$")


def cells(line):
    """The cells of a markdown table row, split on unescaped pipes."""
    return [cell.strip() for cell in PIPE.split(line.strip())[1:-1]]


def expand(paragraphs):
    """'9-11,13' as [9, 10, 11, 13]."""
    result = []
    for part in paragraphs.split(","):
        first, _, last = part.partition("-")
        result.extend(range(int(first), int(last or first) + 1))
    return result


def read_inventory(path, errors):
    """The rows of the one inventory table in path, each with the line it came from."""
    rows = []
    in_table = False
    for lineno, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        where = f"{path.relative_to(ROOT)}:{lineno}"
        if not line.startswith("|"):
            in_table = False
            continue
        row = cells(line)
        if row == COLUMNS:
            in_table = True
            continue
        if not in_table or all(RULE.match(cell) for cell in row):
            continue
        if len(row) != len(COLUMNS):
            errors.append(f"{where}: expected {len(COLUMNS)} cells, found {len(row)}")
            continue
        clause, para, declaration, element, outcome, note = row
        clause = clause.strip("`")
        if not re.fullmatch(r"\[[a-z][a-z0-9_.]*\]", clause):
            errors.append(f"{where}: clause {clause!r} is not a [stable.name]")
            continue
        if not para.isdigit():
            errors.append(f"{where}: paragraph {para!r} is not a number")
            continue
        if outcome not in OUTCOMES:
            errors.append(f"{where}: outcome {outcome!r} is not one of {', '.join(OUTCOMES)}")
            continue
        rows.append({"where": where, "clause": clause[1:-1], "para": int(para), "outcome": outcome, "note": note})
    return rows


def read_citations(directory):
    """Every paragraph cited under directory, as (clause, paragraph) -> the places citing it."""
    cited = defaultdict(list)
    for path in sorted(directory.rglob("*")):
        if path.suffix not in SOURCES:
            continue
        for lineno, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            for match in CITATION.finditer(line):
                for para in expand(match.group(2)):
                    cited[match.group(1), para].append(f"{path.relative_to(ROOT)}:{lineno}")
    return cited


def check(rows, cited, allow_gaps, errors):
    """Append to errors whatever row or citation disagrees with the other side."""
    seen = {}
    for row in rows:
        key = (row["clause"], row["para"])
        name = f"[{row['clause']}]/{row['para']}"
        if key in seen:
            errors.append(f"{row['where']}: {name} is already listed at {seen[key]}")
            continue
        seen[key] = row["where"]
        places = cited.get(key, [])
        if row["outcome"] == "answered" and not places:
            errors.append(f"{row['where']}: {name} is answered, but no test cites it")
        if row["outcome"] in NEEDS_NOTE and not row["note"]:
            errors.append(f"{row['where']}: {name} is {row['outcome']}, and needs a note saying why")
        if row["outcome"] in NEEDS_NOTE and places:
            errors.append(f"{row['where']}: {name} is {row['outcome']}, but {places[0]} cites it")
        if row["outcome"] == "gap" and not allow_gaps:
            errors.append(f"{row['where']}: {name} is a gap")

    # Paragraphs are numbered from 1 without a hole, so a missing row is a missing paragraph.
    by_clause = defaultdict(list)
    for clause, para in seen:
        by_clause[clause].append(para)
    for clause, paras in by_clause.items():
        missing = sorted(set(range(1, max(paras) + 1)) - set(paras))
        if missing:
            errors.append(f"[{clause}]: no row for paragraph {', '.join(map(str, missing))}")

    for (clause, para), places in sorted(cited.items()):
        if clause in by_clause and (clause, para) not in seen:
            for place in places:
                errors.append(f"{place}: [{clause}]/{para} has no inventory row")


def summary(path, rows):
    """One line per clause: answered of paragraphs, then how the rest are accounted for."""
    print(path.relative_to(ROOT))
    clauses = defaultdict(Counter)
    for row in rows:
        clauses[f"[{row['clause']}]"][row["outcome"]] += 1
    clauses["total"] = sum(clauses.values(), Counter())
    width = max(map(len, clauses))
    for clause, count in clauses.items():
        rest = ", ".join(f"{count[outcome]} {outcome}" for outcome in OUTCOMES[1:])
        print(f"  {clause:<{width}}  {count['answered']:>3} of {sum(count.values()):>3} answered ({rest})")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--allow-gaps", action="store_true", help="report gap rows without failing")
    args = parser.parse_args()

    errors = []
    cited = read_citations(ROOT / "test")
    inventories = sorted((ROOT / "doc" / "audit").glob("*.md"))
    rows = []
    for path in inventories:
        these = read_inventory(path, errors)
        if not these:
            errors.append(f"{path.relative_to(ROOT)}: no inventory table")
        summary(path, these)
        rows.extend(these)
    check(rows, cited, args.allow_gaps, errors)

    for error in errors:
        print(f"error: {error}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
