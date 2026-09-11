//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_EXT_INT128_HPP
#define TEST_EXT_INT128_HPP

// The two 128-bit integer classes xstd knows only through an adapter, as Blocks. Neither is a hard dependency,
// so each is detected by the header it would bring rather than by a macro the build must remember to pass --
// which is how xstd-ints detects the same two. They are worth a Block each for a reason the builtin cannot
// cover: the builtin is a scalar the compiler lowers, while these are CLASSES whose operators are ordinary
// functions, so they are the ones that catch a container assuming a Block is a scalar. [design.md#uint128-support]
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
