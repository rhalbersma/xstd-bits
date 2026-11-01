//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>   // bit_array
#include <xstd/bits/bit_utility.hpp> // indeterminate, indeterminate_array, indeterminate_t
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                     // array
#include <concepts>                  // same_as

BOOST_AUTO_TEST_SUITE(Utility)

// The umbrella reaches the tag in one include, and a fixed-width owner built from it takes a value by assignment.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheTag)
{
        static_assert(std::same_as<decltype(xstd::indeterminate), xstd::indeterminate_t const>);
        static_assert(std::same_as<decltype(xstd::indeterminate_array<xstd::bit_array<8>, 2>()), std::array<xstd::bit_array<8>, 2>>);
        auto a = xstd::bit_array<8>(xstd::indeterminate);
        a      = xstd::bit_array<8>{true};
        BOOST_CHECK(a[0] and not a[1]);
}

BOOST_AUTO_TEST_SUITE_END()
