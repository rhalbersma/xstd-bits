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
// And NOT an MSVC-ABI target, which is the term that is easy to drop and costly to lose. There xstd::uint128 is
// the Microsoft STL's std::_Unsigned128 whatever the compiler can do, and clang-cl is the case that proves the
// point: it is clang, so it HAS __int128 and defines __SIZEOF_INT128__, but the alias still names the class. A
// gate that asks only what the compiler supports therefore goes true there and puts a class into word_types,
// which every suite grades over, and into test/src/bits/block/type_traits.cpp, which asserts std::is_integral,
// std::is_unsigned and <bit> of each word. Both are right to assume a builtin, and neither survives a class.
//
// std::_Unsigned128 is a usable Block all the same; test/ext_int128.hpp carries it as one, beside Abseil's and
// Boost's, which is what it behaves like. [design.md#uint128-support]
#if defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__) && !defined(_MSC_VER)
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
