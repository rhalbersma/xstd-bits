//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_WALK_BLOCK_HPP
#define XSTD_BITS_DETAIL_WALK_BLOCK_HPP

#include <xstd/bits/detail/intrinsic.hpp>          // countl_zero, countr_zero
#include <xstd/bits/detail/shift.hpp>              // shl
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cstddef>                                 // size_t

// One block's set positions, the step every walk and every pop takes: report the end, then clear it.
namespace xstd::bits::detail {

// The block with its lowest set position cleared, w & (w - 1); a walk up and pop_front both step by it.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto drop_lowest(Block block) noexcept
        -> Block
{
        return static_cast<Block>(block & static_cast<Block>(block - Block{1}));
}

// The block with its highest set position cleared: w & (w - 1) has no descending twin, so this clears by offset.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto drop_highest(Block block) noexcept
        -> Block
{
        constexpr auto left_bit = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) - 1UZ;
        return static_cast<Block>(block ^ shl(Block{1}, left_bit - countl_zero(block)));
}

// Each set position of one block, lowest first, handed to f as base plus its offset within the block.
template<xstd::unsigned_integer Block, class F>
constexpr auto walk_block_up(Block block, std::size_t base, F& f)
        -> void
{
        while (block != Block{}) {
                f(base + countr_zero(block));
                block = drop_lowest(block);
        }
}

// The mirror, highest first.
template<xstd::unsigned_integer Block, class F>
constexpr auto walk_block_down(Block block, std::size_t base, F& f)
        -> void
{
        constexpr auto left_bit = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) - 1UZ;
        while (block != Block{}) {
                f(base + left_bit - countl_zero(block));
                block = drop_highest(block);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_WALK_BLOCK_HPP
