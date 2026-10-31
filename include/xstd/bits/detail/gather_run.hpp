//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_GATHER_RUN_HPP
#define XSTD_BITS_DETAIL_GATHER_RUN_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/shift.hpp>              // partial_block_mask
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // copy, min
#include <cstddef>                                 // size_t

namespace xstd::bits::detail {

// A run of size positions from start, the slice of stride one: a block at a time through block_at, no permutation.
template<class Src, xstd::unsigned_integer Block>
constexpr auto gather_run(Src const& src, std::size_t start, std::size_t size, borrowed_bits<Block>& dst) noexcept
        -> void
{
        // From a block boundary every whole block is a plain copy into the clear dst, at memmove's speed.
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        std::ranges::copy(src.blocks().subspan(start / block_width<Block>, whole), dst.blocks().begin());
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                dst.block_at(j, src.block_at(start + j), partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j)));
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_GATHER_RUN_HPP
