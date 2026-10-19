//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set.hpp>    // bit_fixed_set, bit_inplace_set, bit_set, bit_set_view, hash
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                    // array
#include <cstdint>                  // uint64_t
#include <functional>               // hash

BOOST_AUTO_TEST_SUITE(BitSetHash)

// Every owner and the view over blocks hash, equal keys equal, whatever holds them.
BOOST_AUTO_TEST_CASE(EverySetHashesItsKeys)
{
        BOOST_CHECK_EQUAL(std::hash<xstd::bit_set>()({1UZ, 64UZ}), std::hash<xstd::bit_set>()({1UZ, 64UZ}));
        BOOST_CHECK(std::hash<xstd::bit_set>()({1UZ}) != std::hash<xstd::bit_set>()({2UZ}));
        BOOST_CHECK_EQUAL(std::hash<xstd::bit_fixed_set<70>>()({3UZ}), std::hash<xstd::bit_fixed_set<70>>()({3UZ}));
        BOOST_CHECK_EQUAL(std::hash<xstd::bit_inplace_set<70>>()({3UZ}), std::hash<xstd::bit_inplace_set<70>>()({3UZ}));
        auto blocks     = std::array<std::uint64_t, 1>{0b1010};
        auto const view = xstd::bit_set_view(blocks);
        BOOST_CHECK_EQUAL(std::hash<decltype(view)>()(view), std::hash<decltype(view)>()(view));
}

BOOST_AUTO_TEST_SUITE_END()
