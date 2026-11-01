//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_FILL_RUN_HPP
#define XSTD_BITS_DETAIL_FILL_RUN_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_ones, block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/shift.hpp>              // partial_block_mask
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // min
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// A scalar at each of the size positions from start, as f combines it with what dst holds.
template<xstd::unsigned_integer Block, class F>
constexpr auto fill_run(bool value, std::size_t start, std::size_t size, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const splat = value ? block_ones<Block> : Block{};
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        for (auto const k : std::views::iota(0UZ, whole)) {
                auto& block = dst[(start / block_width<Block>)+k];
                block       = f(block, splat, block_ones<Block>);
        }
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                auto const mask = partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j));
                auto const pos  = start + j;
                dst.block_at(pos, f(dst.block_at(pos), static_cast<Block>(splat & mask), mask), mask);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_FILL_RUN_HPP
