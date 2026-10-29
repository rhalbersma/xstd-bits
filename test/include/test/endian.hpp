//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ENDIAN_HPP
#define TEST_ENDIAN_HPP

#include <bit> // endian

// An object's bytes are its positions on a little-endian target, the only one where a std::bitset converts by them.
#if defined(_MSC_VER) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#define TEST_BYTES_ARE_POSITIONS

#endif

namespace test {

#ifdef TEST_BYTES_ARE_POSITIONS

inline constexpr bool bytes_are_positions = true;

#else

inline constexpr bool bytes_are_positions = false;

#endif

// The macro agrees with the standard, so a target it misreads fails here rather than skipping or breaking a test.
static_assert(bytes_are_positions == (std::endian::native == std::endian::little));

} // namespace test

#endif // TEST_ENDIAN_HPP
