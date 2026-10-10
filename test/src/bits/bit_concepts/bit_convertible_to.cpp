//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                       // bit_array
#include <xstd/bits/bit_bounded_set.hpp>                 // bit_bounded_set
#include <xstd/bits/bit_concepts/bit_convertible_to.hpp> // bit_convertible_to
#include <xstd/bits/bit_fixed_set.hpp>                   // bit_fixed_set
#include <xstd/bits/bit_set.hpp>                         // bit_set
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_span.hpp>                        // bit_span
#include <xstd/bits/bit_subspan.hpp>                     // bit_subspan
#include <xstd/bits/bit_vector.hpp>                      // bit_vector
#include <xstd/bits/ext/boost/bit_small_vector.hpp>      // bit_small_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <cstdint>                                       // uint32_t, uint64_t, uint8_t
#include <span>                                          // dynamic_extent
#include <vector>                                        // vector

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitConvertibleTo)

// Every width converts: equal fixed widths, a run-time width into a fixed one, and anything into a run-time one.
BOOST_AUTO_TEST_CASE(BitConvertibleToNamesEveryWidthButAViewTarget)
{
        static_assert(xstd::bit_convertible_to<xstd::bit_fixed_set<64>, std::uint64_t> and xstd::bit_convertible_to<std::bitset<20>, xstd::bit_array<20>>);
        static_assert(xstd::bit_convertible_to<std::array<std::uint8_t, 3>, xstd::bit_fixed_set<24>> and xstd::bit_convertible_to<xstd::bit_array<64>, std::bitset<64>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_set_view<std::uint64_t>, std::uint64_t> and xstd::bit_convertible_to<xstd::bit_span<std::array<std::uint8_t, 3>>, xstd::bit_vector>);
        static_assert(xstd::bit_convertible_to<xstd::bit_set, std::uint64_t> and xstd::bit_convertible_to<xstd::bit_vector, std::bitset<70>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_array<64>> and xstd::bit_convertible_to<xstd::bit_bounded_set<64>, xstd::bit_fixed_set<10>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_array<64>, xstd::bit_vector> and xstd::bit_convertible_to<std::bitset<70>, xstd::bit_small_vector<64>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_bounded_set<64>> and xstd::bit_convertible_to<std::uint64_t, xstd::bit_set>);
        static_assert(xstd::bit_convertible_to<xstd::bit_span<std::vector<std::uint64_t>>, xstd::bit_vector> and xstd::bit_convertible_to<xstd::bit_set_view<std::vector<std::uint64_t> const>, xstd::bit_array<64>>);

        // Two fixed widths that differ are no conversion at all, rather than a narrowing or a widening one.
        static_assert(not xstd::bit_convertible_to<std::uint32_t, xstd::bit_array<20>> and not xstd::bit_convertible_to<std::uint64_t, std::bitset<63>>);
        static_assert(not xstd::bit_convertible_to<std::bitset<65>, xstd::bit_array<64>> and not xstd::bit_convertible_to<xstd::bit_fixed_set<64>, xstd::bit_fixed_set<65>>);

        // A window is read from its own first position, as a run-time width.
        static_assert(xstd::bit_convertible_to<xstd::bit_subspan<std::array<std::uint64_t, 2>, std::dynamic_extent, 100>, xstd::bit_vector>);
        static_assert(xstd::bit_convertible_to<xstd::bit_subspan<std::array<std::uint64_t, 2> const, std::dynamic_extent, 100>, xstd::bit_array<100>>);

        // A view is read from and never written into.
        static_assert(not xstd::bit_convertible_to<std::uint64_t, xstd::bit_set_view<std::uint64_t>>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_span<std::array<std::uint8_t, 3>>>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_vector, xstd::bit_subspan<std::array<std::uint64_t, 2>, std::dynamic_extent, 100>>);

        // What has no bit storage converts to nothing.
        static_assert(not xstd::bit_convertible_to<int, xstd::bit_vector> and not xstd::bit_convertible_to<std::vector<bool>, xstd::bit_vector>);
        static_assert(not xstd::bit_convertible_to<xstd::bit_vector, int> and not xstd::bit_convertible_to<xstd::bit_vector, std::vector<std::uint64_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
