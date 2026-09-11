//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_UINT128_HPP
#define TEST_UINT128_HPP

#include <xstd/ints/cstdint/int128.hpp> // IWYU pragma: export; uint128

// Where xstd::uint128 is the compiler's own 128-bit BUILTIN: a scalar, and a std::unsigned_integral. It needs
// __SIZEOF_INT128__ and, to reach <bit> through that concept, a dialect that is not __STRICT_ANSI__ -- the tests
// compile as gnu++, which is why CMAKE_CXX_EXTENSIONS is ON.
//
// Deliberately NOT true on an MSVC-ABI target, where xstd::uint128 is the Microsoft STL's std::_Unsigned128
// instead. That is a usable Block, and test/ext_int128.hpp carries it as one -- but it is a CLASS, so it is not
// std::is_integral, not std::is_unsigned, and not something <bit> will take. This flag feeds word_types, which
// every suite grades over, and test/src/bits/block/type_traits.cpp, which asserts exactly those std traits of
// each word. Both are right to assume a builtin; the integer classes belong in the wide list beside absl's and
// boost's, which is where all three of them are. [design.md#uint128-support]
#if defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__)
#define TEST_HAS_UINT128
#endif

namespace test {

// The macro as a value, for the lists that cannot ask an #if. What holds it honest -- that where it is on the
// Block really has a basis -- is asserted in test/block_types.hpp, with the same check for the integer classes.
// It cannot be asserted here: it has to be written below EVERY adapter, and this header is only one of them.
#ifdef TEST_HAS_UINT128
inline constexpr bool has_uint128 = true;
#else
inline constexpr bool has_uint128 = false;
#endif

} // namespace test

#endif // TEST_UINT128_HPP
