//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>       // perm, piece
#include <xstd/bits/bit_set.hpp>    // bit_bounded_set, bit_enum_set, bit_find_mapping, bit_fixed_set, bit_flag_mapping, bit_flag_set, bit_key_mapping, bit_range_mapping, bit_set, bit_set_view, enum_traits
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                   // bitset
#include <ranges>                   // bidirectional_range
#include <type_traits>              // is_base_of_v

BOOST_AUTO_TEST_SUITE(Set)

// The umbrella reaches every set, the view and the key mappings in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEverySet)
{
        static_assert(std::ranges::bidirectional_range<xstd::bit_fixed_set<8>> and std::ranges::bidirectional_range<xstd::bit_bounded_set<8>> and std::ranges::bidirectional_range<xstd::bit_set>);
        static_assert(std::ranges::bidirectional_range<xstd::bit_enum_set<test::set::perm>> and std::ranges::bidirectional_range<xstd::bit_flag_set<std::bitset<8>>>);
        auto const keys = xstd::bit_fixed_set<8>{1UZ, 5UZ};
        static_assert(std::ranges::bidirectional_range<decltype(xstd::bit_set_view(keys))>);
        static_assert(std::is_base_of_v<xstd::bit_range_mapping<test::set::perm, test::set::perm::read, 3>, xstd::bit_key_mapping<test::set::perm>>);
        static_assert(std::is_base_of_v<xstd::bit_find_mapping<test::set::piece, xstd::enum_traits<test::set::piece>::values>, xstd::bit_key_mapping<test::set::piece>>);
        static_assert(xstd::bit_flag_mapping<std::bitset<8>>::size == 8UZ);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
