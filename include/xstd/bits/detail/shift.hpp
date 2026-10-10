//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SHIFT_HPP
#define XSTD_BITS_DETAIL_SHIFT_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cstddef>                                 // size_t

// A shift count reaches a Block as int, not as the size_t the containers count positions in.
namespace xstd::bits::detail {

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shl(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block << static_cast<int>(n)); // NOLINT(bugprone-signed-bitwise)
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shr(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block >> static_cast<int>(n)); // NOLINT(bugprone-signed-bitwise)
}

// The low count bits of a block, the last of a run of positions; a full block is every bit, with no shift by digits.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto partial_block_mask(std::size_t count) noexcept
        -> Block
{
        constexpr auto digits = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits);
        return count == digits ? static_cast<Block>(~Block{}) : static_cast<Block>(shl(Block{1}, count) - Block{1});
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_SHIFT_HPP
