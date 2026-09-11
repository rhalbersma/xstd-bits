//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <xstd/bits/bit_traits.hpp>   // bit_storage, bit_traits, static_bit_extent
#include <xstd/bits/ext/boost.hpp>    // the Boost adaptors, asked for by name
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <ranges>                     // bidirectional_range
#include <span>                       // dynamic_extent

BOOST_AUTO_TEST_SUITE(Ext)
BOOST_AUTO_TEST_SUITE(Boost)

// As for the std umbrella, asking by name is enough; this is the only adapted type carrying its width in the object.
BOOST_AUTO_TEST_CASE(AskingForItByNameIsEnough)
{
        static_assert(xstd::bit_storage<xstd::bit_traits<boost::dynamic_bitset<>>, boost::dynamic_bitset<>>);
        static_assert(not xstd::static_bit_extent<xstd::bit_traits<boost::dynamic_bitset<>>, boost::dynamic_bitset<>>);
        static_assert(xstd::bit_traits<boost::dynamic_bitset<>>::extent == std::dynamic_extent);
        static_assert(std::ranges::bidirectional_range<xstd::bit_set_view<boost::dynamic_bitset<>>>);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
