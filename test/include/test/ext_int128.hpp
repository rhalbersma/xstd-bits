//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_EXT_INT128_HPP
#define TEST_EXT_INT128_HPP

// The 128-bit integer CLASSES, as Blocks. [design.md#uint128-support]
#ifdef _MSC_VER
#include <xstd/ints/cstdint/int128.hpp> // IWYU pragma: export; uint128
#define TEST_HAS_MSVC_INT128
#endif

#if __has_include(<absl/numeric/int128.h>)
#include <xstd/ints/ext/absl/int128.hpp> // IWYU pragma: export; uint128
#define TEST_HAS_ABSL_INT128
#endif

#if __has_include(<boost/int128.hpp>)
#include <xstd/ints/ext/boost/int128.hpp> // IWYU pragma: export; uint128
#define TEST_HAS_BOOST_INT128
#endif

// As with test/uint128.hpp, the check that these are really usable Blocks lives in test/block_types.hpp.
namespace test {

#ifdef TEST_HAS_MSVC_INT128
inline constexpr bool has_msvc_int128 = true;
#else
inline constexpr bool has_msvc_int128 = false;
#endif

#ifdef TEST_HAS_ABSL_INT128
inline constexpr bool has_absl_int128 = true;
#else
inline constexpr bool has_absl_int128 = false;
#endif

#ifdef TEST_HAS_BOOST_INT128
inline constexpr bool has_boost_int128 = true;
#else
inline constexpr bool has_boost_int128 = false;
#endif

} // namespace test

#endif // TEST_EXT_INT128_HPP
