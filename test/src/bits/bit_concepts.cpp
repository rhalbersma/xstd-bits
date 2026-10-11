//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_concepts.hpp>     // bit_blocks, bit_constructible_from, bit_convertible_to, bit_index_mapping, bit_mask_mapping, sized_bit_index_mapping
#include <xstd/bits/bit_fixed_set.hpp>    // bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp> // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>  // bit_key_mapping
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                          // array
#include <bitset>                         // bitset
#include <cstddef>                        // size_t
#include <cstdint>                        // uint16_t, uint64_t
#include <vector>                         // vector

BOOST_AUTO_TEST_SUITE(BitConcepts)

// The umbrella reaches every concept in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryConcept)
{
        static_assert(xstd::bit_blocks<std::uint64_t> and xstd::bit_blocks<std::array<std::uint16_t, 3>> and xstd::bit_blocks<std::vector<std::size_t>>);
        static_assert(xstd::bit_index_mapping<xstd::bit_key_mapping<std::size_t>, std::size_t>);
        static_assert(xstd::sized_bit_index_mapping<xstd::bit_flag_mapping<std::uint16_t>, std::uint16_t>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::bitset<16>>, std::bitset<16>>);
        static_assert(xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::array<std::uint64_t, 1>>);
        static_assert(xstd::bit_convertible_to<std::bitset<64>, std::uint64_t>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
