//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit.hpp>            // bit_cast, bit_castable
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                       // bitset
#include <cstdint>                      // uint64_t

BOOST_AUTO_TEST_SUITE(Bit)

// The umbrella reaches what <bit> has, extended: one include, and the cast between bit storages is named.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheCast)
{
        static_assert(xstd::bit_castable<std::uint64_t> and xstd::bit_castable<std::bitset<64>>);
        static_assert(xstd::bit_cast<std::uint64_t>(xstd::bit_static_set<64>{0, 63}) == ((1ULL << 63U) | 1ULL));
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
