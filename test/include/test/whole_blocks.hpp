//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_WHOLE_BLOCKS_HPP
#define TEST_WHOLE_BLOCKS_HPP

#include <cstddef> // size_t
#include <limits>  // numeric_limits

// Widths in the library's default block, std::size_t, which is 64 bits on one target and 32 on another.
namespace test {

inline constexpr auto block_bits = static_cast<std::size_t>(std::numeric_limits<std::size_t>::digits);

// A width rounded up to whole default blocks: what a set's universe and an adopted buffer span.
[[nodiscard]] constexpr auto whole_blocks(std::size_t n) noexcept
        -> std::size_t
{
        return (n + block_bits - 1UZ) / block_bits * block_bits;
}

} // namespace test

#endif // TEST_WHOLE_BLOCKS_HPP
