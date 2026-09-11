//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_INPLACE_VECTOR_HPP
#define TEST_INPLACE_VECTOR_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

// The third storage comes and goes with the library, the way xstd::uint128 comes and goes with the compiler. [design.md#growth]
#ifdef __cpp_lib_inplace_vector

#define TEST_HAS_INPLACE_VECTOR
#endif

namespace test {

#ifdef TEST_HAS_INPLACE_VECTOR
inline constexpr bool has_inplace_vector = true;
#else
inline constexpr bool has_inplace_vector = false;
#endif

} // namespace test

#endif // TEST_INPLACE_VECTOR_HPP
