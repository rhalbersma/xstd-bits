//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_UINT128_HPP
#define TEST_UINT128_HPP

#include <xstd/ints/cstdint/int128.hpp> // IWYU pragma: export; uint128
#include <concepts>                     // unsigned_integral
#include <version>                      // _GLIBCXX_RELEASE, _LIBCPP_VERSION

// Where xstd::uint128 is a builtin std::unsigned_integral, which libstdc++ before 16 denies under __STRICT_ANSI__.
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && (!defined(__STRICT_ANSI__) || defined(_LIBCPP_VERSION) || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 16))

#define TEST_HAS_UINT128

#endif

namespace test {

// The macro as a value, for the lists that cannot ask an #if.
#ifdef TEST_HAS_UINT128

inline constexpr bool has_uint128 = true;

#else

inline constexpr bool has_uint128 = false;

#endif

#ifdef XSTD_HAS_INT128

// The gate agrees with the concept, so a standard library that changes its answer fails here rather than skips rows.
static_assert(has_uint128 == std::unsigned_integral<xstd::uint128>);

#endif

} // namespace test

#endif // TEST_UINT128_HPP
