#!/usr/bin/env python3

#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Check that the headers are layered: implementation pushed down, interface pulled up.

A folder under include/xstd/bits/ is one of three kinds. The shared detail/ is the bottom: every
definition more than one folder needs, the block kernels and the readings' adaptors. The layers
above it are the foundation names, <bit>, the iterators, the algorithms and the function objects,
each a public front over detail/ and the layers below it. The container families sit on top, one
folder each, and ext/ beside them builds on whichever it extends.

Five rules hold, read from the #include lines alone:

- detail/ includes no public xstd-bits header, so nothing below a front depends on one;
- no folder reaches back to a folder that reaches it, so the folders stack without a cycle;
- no container family includes another, so deleting one leaves every other building;
- a public header is included by umbrellas and ext/ alone, so deleting one breaks only those;
- a folder's own detail/ is included from that folder alone, so deleting a folder breaks no other.

The umbrellas gather and ext/ extends, so neither is bound by the last three rules. A folder the lists
below do not classify is an error: a new folder says which kind it is before it lands.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BITS = ROOT / "include" / "xstd" / "bits"

SHARED = "detail"
LAYERS = {"bit", "bit_algorithm", "bit_concepts", "bit_functional", "bit_iterator", "bit_ranges", "bit_type_traits", "bit_utility"}
FAMILIES = {"bit_array", "bit_inplace_vector", "bit_set", "bit_span", "bit_valarray", "bit_vector"}
EXTENSIONS = "ext"

INCLUDE = re.compile(r"^#include <xstd/bits/([^>]+)>", re.MULTILINE)


def folder(relative):
    """The folder a header under xstd/bits/ belongs to: its first component, or its own name for an umbrella."""
    parts = relative.split("/")
    return parts[0] if len(parts) > 1 else parts[0].removesuffix(".hpp")


def is_umbrella(relative):
    """An umbrella: a header directly under xstd/bits/, which gathers its folder."""
    return "/" not in relative


def is_detail(relative):
    """A header under a detail/ folder, the shared one or a folder's own."""
    return relative.startswith("detail/") or "/detail/" in relative


def main():
    errors = []
    headers = {path.relative_to(BITS).as_posix(): INCLUDE.findall(path.read_text(encoding="utf-8")) for path in sorted(BITS.rglob("*.hpp"))}
    known = {SHARED, EXTENSIONS} | LAYERS | FAMILIES
    for name in sorted({folder(h) for h in headers} - known):
        errors.append(f"include/xstd/bits/{name}: an unclassified folder; list it as a layer or a family in tools/layering.py")

    edges = {}
    for header, includes in headers.items():
        source = folder(header)
        for included in includes:
            target = folder(included)
            if source == SHARED and target != SHARED:
                errors.append(f"include/xstd/bits/{header}: detail/ includes the public <xstd/bits/{included}>")
            if source in FAMILIES and target in FAMILIES and source != target:
                errors.append(f"include/xstd/bits/{header}: the {source} family includes the {target} family's <xstd/bits/{included}>")
            if source != target and source != EXTENSIONS:
                edges.setdefault(source, set()).add(target)
            exempt = is_umbrella(header) or source == EXTENSIONS
            if not exempt and not is_detail(included) and not is_umbrella(included) and target != EXTENSIONS:
                errors.append(f"include/xstd/bits/{header}: includes the public <xstd/bits/{included}>, which only umbrellas and ext/ include")
            if source != EXTENSIONS and is_detail(included) and target not in (SHARED, source):
                errors.append(f"include/xstd/bits/{header}: includes the {target} folder's own <xstd/bits/{included}>")

    # Tarjan's strongly connected components over the folder graph: any with more than one folder is a cycle.
    index, low, stack, on_stack, cycles = {}, {}, [], set(), []

    def visit(node):
        index[node] = low[node] = len(index)
        stack.append(node)
        on_stack.add(node)
        for successor in sorted(edges.get(node, ())):
            if successor not in index:
                visit(successor)
                low[node] = min(low[node], low[successor])
            elif successor in on_stack:
                low[node] = min(low[node], index[successor])
        if low[node] == index[node]:
            component = []
            while True:
                member = stack.pop()
                on_stack.discard(member)
                component.append(member)
                if member == node:
                    break
            if len(component) > 1:
                cycles.append(sorted(component))

    for node in sorted(edges):
        if node not in index:
            visit(node)
    for cycle in cycles:
        errors.append(f"include/xstd/bits: the folders {', '.join(cycle)} reach one another")

    print(f"{len(headers)} headers, {len(edges)} folders with outgoing includes, {len(errors)} layering errors")
    for error in errors:
        print(f"error: {error}", file=sys.stderr)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
