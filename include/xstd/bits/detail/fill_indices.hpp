//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_FILL_INDICES_HPP
#define XSTD_BITS_DETAIL_FILL_INDICES_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_ones, block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/shift.hpp>              // shl
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// A scalar at index(0), ..., index(count - 1), as f combines it with what dst holds.
template<xstd::unsigned_integer Block, class Index, class F>
constexpr auto fill_indices(bool value, std::size_t count, Index index, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const splat = value ? block_ones<Block> : Block{};
        for (auto const j : std::views::iota(0UZ, count)) {
                auto const pos  = index(j);
                auto const mask = shl(Block{1}, pos % block_width<Block>);
                auto& block     = dst[pos / block_width<Block>];
                block           = f(block, static_cast<Block>(mask & splat), mask);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_FILL_INDICES_HPP
