#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""What each block kernel computes, as the obvious statement about each of its bits.

Each specification is a predicate over the kernel's result, bit by bit: which bit of which input a result bit is, or
that it is clear. None reuses a kernel's shifts or masks, so a proof of a kernel against one is a proof against a
different formulation, not a restatement. Each is written for the Z3 interpretation of blocks.py.
"""


def every_bit(o, predicate):
    return o.and_(*[predicate(i) for i in range(o.width)])


def shifted_left(o, result, block, n):
    """Bit i of the result is bit i - n of the block, and clear below n."""
    return every_bit(o, lambda i: o.bit(result, i) == o.or_(*[o.and_(o.count_eq(n, i - j), o.bit(block, j)) for j in range(i + 1)]))


def shifted_right(o, result, block, n):
    """Bit i of the result is bit i + n of the block, and clear from the width less n."""
    return every_bit(o, lambda i: o.bit(result, i) == o.or_(*[o.and_(o.count_eq(n, j - i), o.bit(block, j)) for j in range(i, o.width)]))


def low_bits(o, result, count):
    """The bits below count set, and no other."""
    return every_bit(o, lambda i: o.bit(result, i) == o.count_ult(i, count))


def stored(o, lo, hi, last, p):
    """Bit p of the two blocks laid end to end, lo first; past lo, a last block has only its clear tail."""
    return o.bit(lo, p) if p < o.width else o.and_(o.not_(last), o.bit(hi, p - o.width))


def read_at(o, result, lo, hi, offset, last):
    """Bit i of the result is bit offset + i of the blocks laid end to end."""
    return every_bit(o, lambda i: o.bit(result, i) == o.or_(
        *[o.and_(o.count_eq(offset, p - i), stored(o, lo, hi, last, p)) for p in range(i, i + o.width)]))


def written_at(o, lo_after, hi_after, lo, hi, offset, last, value, mask):
    """Bit offset + j of the blocks is bit j of value where bit j of mask is set and the bit exists; every other stays."""
    def after(p):
        selects = [(j, o.count_eq(offset, p - j)) for j in range(o.width) if 0 <= p - j < o.width]
        exists = True if p < o.width else o.not_(last)
        written = o.and_(exists, o.or_(*[o.and_(s, o.bit(mask, j)) for j, s in selects], False))
        written_bit = o.or_(*[o.and_(s, o.bit(value, j)) for j, s in selects], False)
        old = o.bit(lo, p) if p < o.width else o.bit(hi, p - o.width)
        new = o.bit(lo_after, p) if p < o.width else o.bit(hi_after, p - o.width)
        return new == o.bool_ite(written, written_bit, old)
    return o.and_(*[after(p) for p in range(2 * o.width)])


def clipped(o, result, block, index, first, last):
    """Bit i of the result is bit i of the block where position index * width + i lies in [first, last), else clear."""
    def at(i):
        position = o.count_add(o.count_mul(index, o.width), i)
        return o.bit(result, i) == o.and_(o.bit(block, i), o.count_ule(first, position), o.count_ult(position, last))
    return every_bit(o, at)


def bitwise(o, result, f, *xs):
    """Bit i of the result is f of bit i of each operand."""
    def at(i):
        bits = [o.bit(x, i) for x in xs]
        if len(bits) == 1:
            answer = o.bool_ite(bits[0], f(True), f(False))
        else:
            answer = o.bool_ite(bits[0], o.bool_ite(bits[1], f(True, True), f(True, False)),
                                o.bool_ite(bits[1], f(False, True), f(False, False)))
        return o.bit(result, i) == answer
    return every_bit(o, at)
