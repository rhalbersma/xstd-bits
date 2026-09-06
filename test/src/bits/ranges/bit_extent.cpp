//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <xstd/bits/basic_bitset.hpp>             // bit_extent over basic_bitset
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_extent over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bit_extent over std::bitset
#include <xstd/bits/ranges/bit_extent.hpp>        // bit_extent, static_bit_extent
#include <bitset>                                 // bitset
#include <span>                                   // dynamic_extent

BOOST_AUTO_TEST_SUITE(Ranges)
BOOST_AUTO_TEST_SUITE(BitExtent)

namespace {

// A type the library knows nothing about, to pin what the primary template says.
struct unknown_to_the_library {};

} // namespace

// Where the width is in the type, the extent is a constant expression and both views can make size() one too.
BOOST_AUTO_TEST_CASE(AWidthInTheTypeIsAStaticExtent)
{
        static_assert(xstd::ranges::bit_extent<std::bitset<  0>> ==   0);
        static_assert(xstd::ranges::bit_extent<std::bitset< 64>> ==  64);
        static_assert(xstd::ranges::bit_extent<xstd::bitset<  0>> ==   0);
        static_assert(xstd::ranges::bit_extent<xstd::bitset<192, unsigned char>> == 192);

        static_assert(xstd::ranges::static_bit_extent<std::bitset<64>>);
        static_assert(xstd::ranges::static_bit_extent<xstd::bitset<64>>);
}

// Where it is in the object, it is not, and both views must ask the object.
BOOST_AUTO_TEST_CASE(AWidthInTheObjectIsADynamicExtent)
{
        static_assert(xstd::ranges::bit_extent<boost::dynamic_bitset<>> == std::dynamic_extent);
        static_assert(not xstd::ranges::static_bit_extent<boost::dynamic_bitset<>>);
}

// The default is dynamic, so a type that has said nothing is asked at run time rather than claiming an extent of zero.
BOOST_AUTO_TEST_CASE(SayingNothingMeansDynamic)
{
        static_assert(xstd::ranges::bit_extent<unknown_to_the_library> == std::dynamic_extent);
        static_assert(not xstd::ranges::static_bit_extent<unknown_to_the_library>);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
