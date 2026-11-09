#!/usr/bin/env python3
#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""Proves each straight-line block kernel equal to its specification for every input, at each block width.

Each claim is a formula Z3 must find no counterexample to: under the kernel's preconditions, its result meets the
specification and every shift it executes is defined. Together they prove, at each width and for every input:

- shl and shr shift by any count below the promoted width, which for a block narrower than int reaches past the
  block's own width, and partial_block_mask sets the low count bits for every count up to the width.
- shr_up_to_width shifts right by any count up to the width, the whole width included.
- block_at reads the width of bits at any offset, from one block or two, and its masked write stores them back.
- masked_block clears what lies outside [first, last) in every block the range touches.
- unary_table and binary_table compute each of the 4 functions of one bool and the 16 of two, bit by bit.

Four mutants, each a kernel with one deliberate error, must be refuted, which shows a passing proof is not vacuous.
One of them, a partial_block_mask without its full-width branch, is refuted only from 32 bits: below that, the block's
promotion to int makes the shift by the width defined and its answer right.

    prove.py [--widths 8,16,32,64,128] [--jobs N] [--ints-proofs DIR]
"""

import argparse
import multiprocessing
import os
import sys
import time

import blocks

operations = None
Z3Blocks = None


def setup(directory):
    global operations, Z3Blocks
    operations = blocks.load(directory)
    Z3Blocks, _ = blocks.interpretations(operations)


import kernels  # noqa: E402
import specifications  # noqa: E402
import z3  # noqa: E402


def with_obligations(o, formula):
    return z3.And(formula, *[condition for _, condition in o.obligations])


def words(o, *names):
    return [z3.BitVec(name, o.width) for name in names]


def counts(*names):
    return [z3.BitVec(name, blocks.COUNT_BITS) for name in names]


def shift_claim(kernel, specification):
    def claim(o):
        (block,) = words(o, "block")
        (n,) = counts("n")
        pre = o.count_ult(n, blocks.promoted_width(o.width))
        return z3.Implies(pre, with_obligations(o, specification(o, kernel(o, block, n, pre), block, n)))
    return claim


def mask_claim(kernel=kernels.partial_block_mask):
    def claim(o):
        (count,) = counts("count")
        pre = o.count_ule(count, o.width)
        return z3.Implies(pre, with_obligations(o, specifications.low_bits(o, kernel(o, count, pre), count)))
    return claim


def mask_without_full_width(o, count, path):
    return kernels.shl(o, 1, count, path) - 1


def shr_up_to_width_claim(o):
    (block,) = words(o, "block")
    (n,) = counts("n")
    pre = o.count_ule(n, o.width)
    return z3.Implies(pre, with_obligations(o, specifications.shifted_right(o, kernels.shr_up_to_width(o, block, n, pre), block, n)))


def read_claim(straddle=kernels.straddled_block):
    def claim(o):
        lo, hi = words(o, "lo", "hi")
        (offset,) = counts("offset")
        last = z3.Bool("last")
        pre = o.count_ult(offset, o.width)
        result = kernels.block_at(o, lo, hi, offset, last, pre, straddle)
        return z3.Implies(pre, with_obligations(o, specifications.read_at(o, result, lo, hi, offset, last)))
    return claim


def straddle_swapped(o, lo, hi, left, right, path):
    return kernels.shl(o, hi, right, path) | kernels.shr(o, lo, left, path)


def write_claim(o):
    lo, hi, value, mask = words(o, "lo", "hi", "value", "mask")
    (offset,) = counts("offset")
    last = z3.Bool("last")
    pre = o.count_ult(offset, o.width)
    lo_after, hi_after = kernels.block_at_write(o, lo, hi, offset, last, value, mask, pre)
    return z3.Implies(pre, with_obligations(o, specifications.written_at(o, lo_after, hi_after, lo, hi, offset, last, value, mask)))


# Positions stay far below 2^64, as no range of blocks reaches it, so index * width + i never wraps.
POSITION_BOUND = 1 << 60


def masked_claim(last_count=None):
    def claim(o):
        (block,) = words(o, "block")
        index, first, last = counts("index", "first", "last")
        pre = z3.And(o.count_ult(first, last), o.count_ule(last, POSITION_BOUND),
                     o.count_ule(o.count_div(first, o.width), index), o.count_ule(index, o.count_div(o.count_sub(last, 1), o.width)))
        result = kernels.masked_block(o, block, index, first, last, pre, last_count)
        return z3.Implies(pre, with_obligations(o, specifications.clipped(o, result, block, index, first, last)))
    return claim


def table(arity, index):
    """The function of arity bools whose truth table is the bits of index, argument (x, y) at bit 2x + y."""
    if arity == 1:
        return lambda x: bool((index >> int(x)) & 1)
    return lambda x, y: bool((index >> (2 * int(x) + int(y))) & 1)


def unary_claim(index):
    def claim(o):
        (x,) = words(o, "x")
        f = table(1, index)
        return specifications.bitwise(o, kernels.unary_table(o, f, x), f, x)
    return claim


def binary_claim(index, swap_mixed=False):
    def claim(o):
        x, y = words(o, "x", "y")
        f = table(2, index)
        return specifications.bitwise(o, kernels.binary_table(o, f, x, y, swap_mixed), f, x, y)
    return claim


def claims(width):
    """Every claim at this width, in order, as (name, builder, expected to hold)."""
    found = [
        ("shl shifts left by any count below the promoted width", shift_claim(kernels.shl, specifications.shifted_left), True),
        ("shr shifts right by any count below the promoted width", shift_claim(kernels.shr, specifications.shifted_right), True),
        ("partial_block_mask sets the low count bits, up to the width", mask_claim(), True),
        ("shr_up_to_width shifts right by any count up to the width", shr_up_to_width_claim, True),
        ("block_at reads the width of bits at any offset", read_claim(), True),
        ("block_at's masked write stores them back", write_claim, True),
        ("masked_block clears what lies outside [first, last)", masked_claim(), True),
    ]
    found += [(f"unary_table computes the function of one bool with truth table {i:02b}", unary_claim(i), True) for i in range(4)]
    found += [(f"binary_table computes the function of two bools with truth table {i:04b}", binary_claim(i), True) for i in range(16)]
    found += [
        ("mutant: a straddled read with its two shift counts swapped", read_claim(straddle_swapped), False),
        ("mutant: a masked_block keeping last % width bits of its last block", masked_claim(lambda o, last: o.count_mod(last, o.width)), False),
        ("mutant: a binary_table with its two mixed rows swapped", binary_claim(0b0100, swap_mixed=True), False),
        ("mutant: a partial_block_mask without its full-width branch", mask_claim(mask_without_full_width), width < blocks.INT_BITS),
    ]
    return found


def solve(task):
    """One claim at one width, in a process of its own: whether it holds, how long it took, and any counterexample."""
    directory, width, index = task
    setup(directory)
    name, build, expected = claims(width)[index]
    o = Z3Blocks(width)
    formula = build(o)
    solver = z3.SolverFor("QF_BV")
    solver.add(z3.Not(formula))
    start = time.monotonic()
    result = solver.check()
    elapsed = time.monotonic() - start
    detail = ""
    if result == z3.sat:
        model = solver.model()
        detail = ", ".join(f"{d.name()} = {model[d]}" for d in sorted(model.decls(), key=str))
        broken = [what for what, condition in o.obligations if z3.is_false(model.eval(condition, model_completion=True))]
        detail += f"; breaks {broken}" if broken else ""
    return width, name, expected, str(result), elapsed, detail


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--widths", default="8,16,32,64,128", help="comma-separated block widths, from 8 to 128")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 1)
    parser.add_argument("--ints-proofs", default=os.environ.get("XSTD_INTS_PROOFS"), help="xstd-ints' test/proofs directory")
    args = parser.parse_args()
    setup(args.ints_proofs)
    tasks = [(args.ints_proofs, width, index) for width in (int(w) for w in args.widths.split(",")) for index in range(len(claims(width)))]
    as_expected = True
    with multiprocessing.get_context("spawn").Pool(args.jobs) as pool:
        for width, name, expected, result, elapsed, detail in pool.imap(solve, tasks):
            holds = result == "unsat"
            verdict = "proved" if holds else ("refuted" if result == "sat" else result)
            ok = holds == expected and result in ("sat", "unsat")
            as_expected &= ok
            shown = detail if (detail and not ok) or not expected else ""
            print(f"{width:>3} bits  {verdict:<8} {elapsed:7.1f} s  {name}" + (f": {shown}" if shown else "") + ("" if ok else "  <-- not as expected"), flush=True)
    return 0 if as_expected else 1


if __name__ == "__main__":
    sys.exit(main())
