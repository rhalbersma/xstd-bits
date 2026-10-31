//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_EXPAND_INDICES_HPP
#define XSTD_BITS_DETAIL_EXPAND_INDICES_HPP

#include <xstd/bits/detail/bit_at.hpp>             // bit_at
#include <xstd/bits/detail/block_width.hpp>        // block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/shift.hpp>              // shl
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// One position at a time and in order, so a position named twice takes what f makes of its later bit.
template<class Src, xstd::unsigned_integer Block, class Index, class F>
constexpr auto expand_indices(Src const& src, std::size_t count, Index index, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, count)) {
                // One position is a block whose mask has one bit, which the block combiner takes as it takes any other.
                auto const pos  = index(j);
                auto const mask = shl(Block{1}, pos % block_width<Block>);
                auto& block     = dst[pos / block_width<Block>];
                block           = f(block, shl(bit_at(src, j), pos % block_width<Block>), mask);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_EXPAND_INDICES_HPP
