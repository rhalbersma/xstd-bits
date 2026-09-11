//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_UINT128_HPP
#define TEST_UINT128_HPP

#include <xstd/ints/cstdint/int128.hpp> // IWYU pragma: export; uint128

// Where xstd::uint128 is a usable Block. Two spellings, because xstd::uint128 is two different types: on an
// MSVC-ABI target (clang-cl included) it is the Microsoft STL's std::_Unsigned128, a CLASS, which is always
// there and now carries its own bit basis; everywhere else it is the compiler's own extension, which needs
// __SIZEOF_INT128__ and, to reach <bit> through std::unsigned_integral, a dialect that is not __STRICT_ANSI__ --
// the tests compile as gnu++, which is why CMAKE_CXX_EXTENSIONS is ON. [design.md#uint128-support]
#if defined(_MSC_VER) || (defined(__SIZEOF_INT128__) && !defined(__STRICT_ANSI__))
#define TEST_HAS_UINT128
#endif

namespace test {

// The macro as a value, for the lists that cannot ask an #if. What holds it honest -- that where it is on the
// Block really has a basis -- is asserted in test/block_types.hpp, with the same check for the other two. It
// cannot be asserted here: it has to be written below EVERY adapter, and this header is only one of them.
#ifdef TEST_HAS_UINT128
inline constexpr bool has_uint128 = true;
#else
inline constexpr bool has_uint128 = false;
#endif

} // namespace test

#endif // TEST_UINT128_HPP
