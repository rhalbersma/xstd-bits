//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_UINT128_HPP
#define TEST_UINT128_HPP

#include <xstd/ints/cstdint/int128.hpp> // IWYU pragma: export; uint128

// Where xstd::uint128 is the compiler's own 128-bit BUILTIN: a scalar, and a std::unsigned_integral. [design.md#uint128-support]
#if defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__) && !defined(_MSC_VER)
#define TEST_HAS_UINT128
#endif

namespace test {

// The macro as a value, for the lists that cannot ask an #if.
#ifdef TEST_HAS_UINT128
inline constexpr bool has_uint128 = true;
#else
inline constexpr bool has_uint128 = false;
#endif

} // namespace test

#endif // TEST_UINT128_HPP
