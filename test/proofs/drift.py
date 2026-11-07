#!/usr/bin/env python3
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Holds the models the proofs are about against the compiled block kernels they transcribe.

A proof is about kernels.py, not about the C++. This runs the compiled kernels, through drift.cpp, on edge values and
on pseudo-random blocks at every block width the executable has, and evaluates the models on the same inputs, so a
kernel edited without its model, or a model that misreads its kernel, fails here.

    drift.py path/to/proofs_drift [--ints-proofs DIR]
"""

import argparse
import os
import subprocess
import sys

import blocks
import kernels


def splitmix(state):
    """SplitMix64: the next state and its output, so the inputs are the same on every run."""
    state = (state + 0x9E3779B97F4A7C15) % 2**64
    z = state
    z = ((z ^ (z >> 30)) * 0xBF58476D1CE4E5B9) % 2**64
    z = ((z ^ (z >> 27)) * 0x94D049BB133111EB) % 2**64
    return state, z ^ (z >> 31)


def edges(width):
    """Zero, all ones, and at every position the single bit, its complement, and the masks below and from it."""
    ones = (1 << width) - 1
    values = {0, ones}
    for i in range(width):
        values |= {1 << i, ones ^ (1 << i), (1 << i) - 1, ones ^ ((1 << i) - 1)}
    return sorted(values)


def inputs(width, samples=4000):
    """Every edge value against every count up to twice the width, then pseudo-random blocks and counts."""
    ones = (1 << width) - 1
    rows = []
    for x in edges(width):
        for n in range(2 * width + 1):
            rows.append((x, ones ^ x, x, ones, n, n * 7))
    state = 0
    for _ in range(samples):
        draws = []
        for _ in range(6):
            state, z = splitmix(state)
            draws.append(z)
        wide = [(draws[i] | (draws[(i + 1) % 6] << 64)) & ones for i in range(4)]
        rows.append((wide[0], wide[1], wide[2], wide[3], draws[4], draws[5]))
    return rows


def modelled(o, row):
    width = o.width
    x, y, value, mask, n, m = row
    x, y, value, mask = (o.word(v) for v in (x, y, value, mask))
    stored = (x, y)
    out = [kernels.shl(o, x, n % blocks.promoted_width(width), True), kernels.shr(o, x, n % blocks.promoted_width(width), True),
           kernels.partial_block_mask(o, n % (width + 1), True), kernels.shr_up_to_width(o, x, n % (width + 1), True)]

    p = n % (2 * width)
    index, offset, last = p // width, p % width, p >= width
    hi = stored[1] if index == 0 else o.word(0)
    out.append(kernels.block_at(o, stored[index], hi, offset, last, True))
    inner = mask if p <= width else mask & kernels.partial_block_mask(o, 2 * width - p, True)
    lo_after, hi_after = kernels.block_at_write(o, stored[index], hi, offset, last, value, inner, True)
    out += [lo_after, hi_after] if index == 0 else [x, lo_after]

    first = p
    last_position = first + 1 + (m % (2 * width - first))
    for at in (first // width, (last_position - 1) // width):
        out.append(kernels.masked_block(o, stored[at], at, first, last_position, True))

    out.append(kernels.unary_table(o, lambda v, i=m % 4: bool((i >> int(v)) & 1), x))
    out.append(kernels.binary_table(o, lambda u, v, i=m % 16: bool((i >> (2 * int(u) + int(v))) & 1), x, y))
    return [format(v.value, "x") for v in out]


NAMES = ("shl", "shr", "partial_block_mask", "shr_up_to_width", "block_at", "block_at write, low block",
         "block_at write, high block", "masked_block, first block", "masked_block, last block", "unary_table", "binary_table")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("executable")
    parser.add_argument("--ints-proofs", default=os.environ.get("XSTD_INTS_PROOFS"), help="xstd-ints' test/proofs directory")
    args = parser.parse_args()
    _, IntBlocks = blocks.interpretations(blocks.load(args.ints_proofs))
    widths = [int(w) for w in subprocess.run([args.executable, "--widths"], capture_output=True, text=True, check=True).stdout.split()]
    status = 0
    for width in widths:
        rows = inputs(width)
        feed = "".join(" ".join(format(v, "x") for v in row) + "\n" for row in rows)
        compiled = subprocess.run([args.executable, str(width)], input=feed, capture_output=True, text=True, check=True).stdout.splitlines()
        if len(compiled) != len(rows):
            sys.stderr.write(f"{width} bits: the kernels answered {len(compiled)} of {len(rows)} rows\n")
            status = 1
            continue
        o = IntBlocks(width)
        drifted = 0
        for row, line in zip(rows, compiled):
            for name, got, want in zip(NAMES, line.split(), modelled(o, row)):
                if got != want:
                    drifted += 1
                    if drifted <= 10:
                        sys.stderr.write(f"{width} bits, {name} on {tuple(hex(v) for v in row)}: compiled {got}, modelled {want}\n")
        print(f"{width:>3}-bit kernels and models agree on all {len(rows)} rows" if not drifted else f"{width:>3} bits: {drifted} outputs differ")
        status |= drifted != 0
    return status


if __name__ == "__main__":
    sys.exit(main())
