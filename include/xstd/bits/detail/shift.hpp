//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SHIFT_HPP
#define XSTD_BITS_DETAIL_SHIFT_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

// A shift count reaches a Block as int, not as the size_t the containers count positions in. [design.md#uint128-support] [design.md#clang-tidy-false-positives]
namespace xstd::detail::bits {

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shl(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block << static_cast<int>(n));  // NOLINT(bugprone-signed-bitwise)
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shr(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block >> static_cast<int>(n));  // NOLINT(bugprone-signed-bitwise)
}

}       // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_SHIFT_HPP
