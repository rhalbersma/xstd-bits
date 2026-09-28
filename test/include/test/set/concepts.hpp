//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_CONCEPTS_HPP
#define TEST_SET_CONCEPTS_HPP

#include <test/value_reference.hpp> // value_reference
#include <concepts>                 // regular, totally_ordered
#include <iterator>                 // bidirectional_iterator
#include <ranges>                   // bidirectional_range

namespace test::set {

// The interface a bit-packed set shares with the set it packs: bidirectional over keys, not random-access over bool.
template<class C>
concept bit_set =
        std::regular<C> and std::totally_ordered<C> and std::ranges::bidirectional_range<C> and std::bidirectional_iterator<typename C::iterator> and value_reference<typename C::const_reference>;

} // namespace test::set

#endif // TEST_SET_CONCEPTS_HPP
