//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>  // bit_array, to_bit_array
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <functional>               // hash
#include <tuple>                    // tuple_size_v

BOOST_AUTO_TEST_SUITE(BitArrayUmbrella)

// The umbrella reaches the container and every free part of its surface in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheWholeSurface)
{
        using T = xstd::bit_array<3>;
        static_assert(std::tuple_size_v<T> == 3UZ);
        BOOST_CHECK(xstd::to_bit_array({true, false, true}) == T({true, false, true}));
        BOOST_CHECK_EQUAL(std::hash<T>()(T({true, false, true})), std::hash<T>()(T({true, false, true})));
}

BOOST_AUTO_TEST_SUITE_END()
