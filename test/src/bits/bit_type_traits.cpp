//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>       // basic_bit_array
#include <xstd/bits/bit_flag_set.hpp>    // bit_flag_set
#include <xstd/bits/bit_type_traits.hpp> // bit_align, bit_blocks_capacity_v, bit_blocks_extent_v, bit_fast, bit_least, bit_underlying, fast_block_t, least_block_t, underlying_block_t
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                         // array
#include <cstdint>                       // int16_t, uint16_t, uint64_t, uint8_t, uint_fast16_t
#include <type_traits>                   // is_same_v

BOOST_AUTO_TEST_SUITE(BitTypeTraits)

// The umbrella reaches every width and transformation in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryTransformation)
{
        static_assert(xstd::bit_blocks_extent_v<std::array<std::uint64_t, 2>> == 128 and xstd::bit_blocks_capacity_v<std::uint64_t> == 64);
        static_assert(std::is_same_v<xstd::least_block_t<9>, std::uint16_t> and std::is_same_v<xstd::fast_block_t<9>, std::uint_fast16_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<std::int16_t>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::bit_least<xstd::basic_bit_array<std::uint64_t, 9>>, xstd::basic_bit_array<std::uint16_t, 9>>);
        static_assert(std::is_same_v<xstd::bit_fast<xstd::basic_bit_array<std::uint64_t, 9>>, xstd::basic_bit_array<std::uint_fast16_t, 9>>);
        static_assert(std::is_same_v<xstd::bit_align<xstd::basic_bit_array<std::uint8_t, 9>>, xstd::basic_bit_array<std::uint8_t, 16>>);
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::bit_flag_set<std::int16_t>>, xstd::bit_flag_set<std::int16_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
