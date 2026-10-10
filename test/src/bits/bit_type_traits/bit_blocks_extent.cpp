//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp>                      // bit_set_view
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                           // array
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint16_t, uint32_t, uint64_t, uint8_t
#include <span>                                            // dynamic_extent, span
#include <type_traits>                                     // is_same_v
#include <vector>                                          // vector

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitBlocksExtent)

namespace {

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks        = std::uint64_t[4];       // NOLINT(modernize-avoid-c-arrays): the storage under test
using three_const_blocks = std::uint16_t const[3]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// The width storage names by its type, which the views default to: fixed blocks have one, the rest do not.
BOOST_AUTO_TEST_CASE(TheExtentIsTheWidthTheTypeNames)
{
        static_assert(xstd::bit_blocks_extent_v<std::uint8_t> == 8 and xstd::bit_blocks_extent_v<std::uint64_t const> == 64);
        static_assert(xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3>> == 48 and xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3> const> == 48);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t, 2>> == 64 and xstd::bit_blocks_extent_v<std::span<std::uint32_t const, 2>> == 64);
        static_assert(xstd::bit_blocks_extent_v<four_blocks> == 256 and xstd::bit_blocks_extent_v<three_const_blocks> == 48);
        static_assert(xstd::bit_blocks_extent_v<four_blocks> == xstd::bit_blocks_extent_v<std::array<std::uint64_t, 4>>);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t>> == std::dynamic_extent);
        static_assert(xstd::bit_blocks_extent_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bit_set_view<std::span<std::uint32_t>>, xstd::bit_set_view<std::span<std::uint32_t>, std::dynamic_extent>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
