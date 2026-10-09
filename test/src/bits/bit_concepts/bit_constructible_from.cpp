//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                           // bit_array
#include <xstd/bits/bit_bounded_set.hpp>                     // bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>                  // bit_bounded_vector
#include <xstd/bits/bit_concepts/bit_constructible_from.hpp> // bit_constructible_from
#include <xstd/bits/bit_fixed_set.hpp>                       // bit_fixed_set
#include <xstd/bits/bit_set.hpp>                             // bit_set
#include <xstd/bits/bit_set_view.hpp>                        // bit_set_view
#include <xstd/bits/bit_vector.hpp>                          // bit_vector
#include <boost/test/unit_test.hpp>                          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                             // array
#include <bitset>                                            // bitset
#include <cstddef>                                           // size_t
#include <cstdint>                                           // uint64_t, uint8_t
#include <vector>                                            // vector

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitConstructibleFrom)

// Blocks an owner takes as they are: its own container at a run-time width, or a field at a fixed one.
BOOST_AUTO_TEST_CASE(BitConstructibleFromNamesBlocksTakenAsTheyAre)
{
        static_assert(xstd::bit_constructible_from<xstd::bit_vector, std::vector<std::size_t>> and xstd::bit_constructible_from<xstd::bit_set, std::vector<std::size_t>>);
        static_assert(xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::array<std::uint64_t, 1>> and xstd::bit_constructible_from<xstd::bit_array<20>, std::array<std::uint8_t, 3>>);
        static_assert(xstd::bit_constructible_from<xstd::bit_bounded_set<100>, xstd::bit_bounded_set<100>::block_container_type>);

        // Another block type, a field too narrow, and a container a bounded owner does not hold are none of those.
        static_assert(not xstd::bit_constructible_from<xstd::bit_vector, std::vector<std::uint8_t>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_fixed_set<256>, std::array<std::uint64_t, 3>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_bounded_set<100>, std::array<std::uint64_t, 2>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_bounded_vector<100>, std::vector<std::size_t>>);

        // What has bit storage without being it, and a view, which owns nothing to take blocks into.
        static_assert(not xstd::bit_constructible_from<xstd::bit_fixed_set<64>, std::bitset<64>>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_vector, xstd::bit_set>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_set_view<std::uint64_t>, std::uint64_t>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
