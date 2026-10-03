//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit.hpp>           // bit_constructible_from, bit_convert, bit_convertible_to
#include <xstd/bits/bit_fixed_set.hpp> // bit_fixed_set
#include <boost/test/unit_test.hpp>    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                       // array
#include <bitset>                      // bitset
#include <cstdint>                     // uint64_t

BOOST_AUTO_TEST_SUITE(Bit)

// The umbrella reaches the conversion and both of its concepts in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheConversion)
{
        static_assert(xstd::bit_convertible_to<std::bitset<64>, std::uint64_t>);
        static_assert(xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::array<std::uint64_t, 1>>);
        static_assert(xstd::bit_convert<std::uint64_t>(xstd::bit_fixed_set<64>{0, 63}) == ((1ULL << 63U) | 1ULL));
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
