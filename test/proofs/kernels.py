#          Copyright Rein Halbersma 2014-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

"""The straight-line block kernels, transcribed statement by statement, at any block width.

Each kernel is written over the operations of blocks.py: words at the block's width and counts at std::size_t's. A
kernel takes the path that reaches it and records, as an obligation, what the C++ needs on that path to be defined: a
shift by a count below the promoted width. A branch narrows the path its arms see, as the C++ evaluates one arm only.

- shl, shr and partial_block_mask: include/xstd/bits/detail/shift.hpp
- shr_up_to_width: include/xstd/bits/detail/bit_stream.hpp
- block_at, its read and its masked write, and straddled_block: include/xstd/bits/detail/bit_block_container.hpp
- masked_block: include/xstd/bits/bit_iterator/detail/select.hpp
- unary_table and binary_table: include/xstd/bits/bit_algorithm/detail/blit.hpp
"""

import blocks


def ones(o):
    return o.word((1 << o.width) - 1)


def shl(o, block, n, path):
    o.require(path, o.count_ult(n, blocks.promoted_width(o.width)), "shl: the count below the promoted width")
    return o.shl_by(block, n)


def shr(o, block, n, path):
    o.require(path, o.count_ult(n, blocks.promoted_width(o.width)), "shr: the count below the promoted width")
    return o.shr_by(block, n)


def partial_block_mask(o, count, path):
    full = o.count_eq(count, o.width)
    return o.ite(full, ones(o), shl(o, 1, count, o.and_(path, o.not_(full))) - 1)


def shr_up_to_width(o, block, n, path):
    half = o.count_div(n, 2)
    return shr(o, shr(o, block, half, path), o.count_sub(n, half), path)


def straddled_block(o, lo, hi, left, right, path):
    return shl(o, hi, left, path) | shr(o, lo, right, path)


def block_at(o, lo, hi, offset, last, path, straddle=straddled_block):
    """The read at position index * width + offset, lo the block at index and hi the next, which last says is absent."""
    whole = o.or_(o.count_eq(offset, 0), last)
    return o.ite(whole, shr(o, lo, offset, o.and_(path, whole)),
                 straddle(o, lo, hi, o.count_sub(o.width, offset), offset, o.and_(path, o.not_(whole))))


def block_at_write(o, lo, hi, offset, last, value, mask, path):
    """The masked write at the same position: lo and hi as they stand after it."""
    bits = value & mask
    low_kept = lo & ~shl(o, mask, offset, path)
    lo_after = low_kept | shl(o, bits, offset, path)
    straddles = o.and_(o.not_(o.count_eq(offset, 0)), o.not_(last))
    shift = o.count_sub(o.width, offset)
    on = o.and_(path, straddles)
    high_kept = hi & ~shr(o, mask, shift, on)
    return lo_after, o.ite(straddles, high_kept | shr(o, bits, shift, on), hi)


def masked_block(o, block, index, first, last, path, last_count=None):
    """The block at index with what lies outside [first, last) cleared."""
    first_block = o.count_eq(index, o.count_div(first, o.width))
    block = o.ite(first_block, block & ~partial_block_mask(o, o.count_mod(first, o.width), o.and_(path, first_block)), block)
    last_block = o.count_eq(index, o.count_div(o.count_sub(last, 1), o.width))
    count = (last_count or (lambda o, last: o.count_add(o.count_mod(o.count_sub(last, 1), o.width), 1)))(o, last)
    return o.ite(last_block, block & partial_block_mask(o, count, o.and_(path, last_block)), block)


def everything_or_nothing(o, truth):
    return ones(o) if truth else o.word(0)


def unary_table(o, f, x):
    """f as its answers at false and at true, then each bit of x through them."""
    if_false, if_true = everything_or_nothing(o, f(False)), everything_or_nothing(o, f(True))
    return (x & if_true) | (~x & if_false)


def binary_table(o, f, x, y, swap_mixed=False):
    """f as its truth table, asked at the four pairs, then each pair of bits of x and y through it."""
    t00, t11 = everything_or_nothing(o, f(False, False)), everything_or_nothing(o, f(True, True))
    t01, t10 = everything_or_nothing(o, f(False, True)), everything_or_nothing(o, f(True, False))
    if swap_mixed:
        t01, t10 = t10, t01
    nx, ny = ~x, ~y
    return ((x & y) & t11) | ((x & ny) & t10) | ((nx & y) & t01) | ((nx & ny) & t00)
