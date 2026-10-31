//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_EXPAND_RUN_HPP
#define XSTD_BITS_DETAIL_EXPAND_RUN_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_ones, block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/scatter.hpp>            // scatter_assign
#include <xstd/bits/detail/shift.hpp>              // partial_block_mask
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // copy, min, transform
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <type_traits>                             // remove_const_t

namespace xstd::bits::detail {

// The run of size positions from start takes the bits of src in order, as f combines them with what dst holds.
template<class Src, xstd::unsigned_integer Block, class F>
constexpr auto expand_run(Src const& src, std::size_t start, std::size_t size, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        auto const from  = src.blocks().first(whole);
        auto const onto  = dst.blocks().subspan(start / block_width<Block>, whole);
        if constexpr (std::same_as<F, std::remove_const_t<decltype(scatter_assign)>>) {
                // Whole blocks assigned are a copy at memmove's speed, and nothing at all onto themselves.
                if (from.data() != onto.data()) {
                        std::ranges::copy(from, onto.begin());
                }
        } else {
                std::ranges::transform(onto, from, onto.begin(), [&](Block d, Block e) noexcept -> Block { return f(d, e, block_ones<Block>); });
        }
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                auto const mask = partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j));
                auto const pos  = start + j;
                dst.block_at(pos, f(dst.block_at(pos), static_cast<Block>(src.block_at(j) & mask), mask), mask);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_EXPAND_RUN_HPP
