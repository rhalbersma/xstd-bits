//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                       // bit_array
#include <xstd/bits/bit_concepts/bit_convertible.hpp>    // bit_convertible
#include <xstd/bits/bit_concepts/bit_convertible_to.hpp> // bit_convertible_to
#include <xstd/bits/bit_set.hpp>                         // bit_set
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_vector.hpp>                      // bit_vector
#include <xstd/bits/ext/boost/bit_small_vector.hpp>      // bit_small_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                                        // bitset
#include <cstdint>                                       // uint32_t, uint64_t

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitConvertible)

// The constraint is on the two types as they are declared: a reference or a const source is the same conversion.
BOOST_AUTO_TEST_CASE(BitConvertibleConstrainsTheTypesNotTheExpression)
{
        static_assert(xstd::bit_convertible<xstd::bit_array<64>, std::uint64_t> and xstd::bit_convertible<xstd::bit_vector, xstd::bit_array<64>>);
        static_assert(xstd::bit_convertible<xstd::bit_set, xstd::bit_vector> and xstd::bit_convertible<std::bitset<70>, xstd::bit_small_vector<64>>);
        static_assert(not xstd::bit_convertible<std::uint32_t, xstd::bit_array<20>> and not xstd::bit_convertible<std::uint64_t, xstd::bit_set_view<std::uint64_t>>);
        static_assert(xstd::bit_convertible_to<xstd::bit_set const&, xstd::bit_vector> and xstd::bit_convertible<xstd::bit_set, xstd::bit_vector>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
