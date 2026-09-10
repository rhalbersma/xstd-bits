//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_FLAT_SET_HPP
#define TEST_FLAT_SET_HPP

#include <version>  // __cpp_lib_flat_set
#ifdef __cpp_lib_flat_set
#define TEST_HAS_FLAT_SET
#include <flat_set> // IWYU pragma: export; flat_set
#endif

// VS 2022's MSVC STL has no <flat_set>; the probe lives here because <version> has to precede it.
namespace test {

template<class T>
inline constexpr bool is_flat_set = false;

#ifdef TEST_HAS_FLAT_SET

template<class Key, class Compare, class KeyContainer>
inline constexpr bool is_flat_set<std::flat_set<Key, Compare, KeyContainer>> = true;

#endif

} // namespace test

#endif // TEST_FLAT_SET_HPP
