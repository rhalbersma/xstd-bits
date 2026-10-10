//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_ORDER_ISOMORPHISM_HPP
#define TEST_SET_ORDER_ISOMORPHISM_HPP

#include <xstd/bits/detail/is_key.hpp> // is_key
#include <concepts>                    // totally_ordered
#include <cstddef>                     // size_t
#include <ranges>                      // iota

// What makes a mapped set a std::set: its mapping is an order isomorphism from its keys onto the first positions.
namespace test::set {

// Each of positions 0 to n - 1 holds a key that ranks back to it, and where keys have an order, above the one before.
template<class Mapping, class Key>
[[nodiscard]] constexpr auto is_order_isomorphism_onto(std::size_t n)
        -> bool
{
        for (auto const i : std::views::iota(0UZ, n)) {
                auto const key = Mapping::from_index(i);
                if (not xstd::bits::detail::is_key<Mapping>(key) or Mapping::to_index(key) != i) {
                        return false;
                }
                if constexpr (std::totally_ordered<Key>) {
                        if (i != 0UZ and not(Mapping::from_index(i - 1UZ) < key)) {
                                return false;
                        }
                }
        }
        return true;
}

// A sized mapping onto all of its positions.
template<class Mapping, class Key>
[[nodiscard]] constexpr auto is_order_isomorphism()
        -> bool
{
        return is_order_isomorphism_onto<Mapping, Key>(Mapping::size);
}

} // namespace test::set

#endif // TEST_SET_ORDER_ISOMORPHISM_HPP
