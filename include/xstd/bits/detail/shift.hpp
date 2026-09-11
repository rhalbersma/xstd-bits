//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SHIFT_HPP
#define XSTD_BITS_DETAIL_SHIFT_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

// A shift count reaches a Block as int, not as the size_t the containers count positions in. A builtin Block
// takes it either way, but an integer CLASS declares its shift with a fixed parameter -- absl::uint128's is
// int -- so a size_t narrows there and -Wconversion rejects it. Every count in this library is a bit position
// within one block, so it is always far inside int's range.
//
// Said once here rather than at each of the two dozen shift sites: a cast repeated that many times is a cast
// that will be forgotten at the next one, and forgetting it breaks only the integer-class Blocks, on only the
// builds that have them -- which is how the first count of these sites came out too low. The narrowing cast back to Block is the other half of every one of those sites, so it
// belongs here too. Named shl/shr because this header's callers already use std::shift_left and
// std::shift_right for the range algorithms. [design.md#uint128-support]
namespace xstd::detail::bits {

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shl(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block << static_cast<int>(n));
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shr(Block block, std::size_t n) noexcept
        -> Block
{
        return static_cast<Block>(block >> static_cast<int>(n));
}

}       // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_SHIFT_HPP
