//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <xstd/bits/bit_traits.hpp>      // bit_storage, bit_traits, static_bit_extent
#include <xstd/bits/ext/std.hpp>         // the std adaptors, asked for by name
#include <xstd/bits/ranges/set_view.hpp> // set_view
#include <bitset>                        // bitset
#include <ranges>                        // bidirectional_range

BOOST_AUTO_TEST_SUITE(Ext)
BOOST_AUTO_TEST_SUITE(Std)

// One adapted library and no umbrella above it, so the umbrella never puts someone else's headers on a consumer's path.
BOOST_AUTO_TEST_CASE(AskingForItByNameIsEnough)
{
        static_assert(xstd::bit_storage<xstd::bit_traits<std::bitset<8>>, std::bitset<8>>);
        static_assert(xstd::static_bit_extent<xstd::bit_traits<std::bitset<8>>, std::bitset<8>>);
        static_assert(xstd::bit_traits<std::bitset<8>>::extent == 8);
        static_assert(std::ranges::bidirectional_range<xstd::set_view<std::bitset<8>>>);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
