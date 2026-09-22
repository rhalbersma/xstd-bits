//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/tags.hpp> // array_container_tag, bitset_reading_tag, container_tag, inplace_vector_container_tag, reading_tag, sequence_reading_tag, set_reading_tag, vector_container_tag
#include <test/inplace_vector.hpp>   // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Tags)

BOOST_AUTO_TEST_CASE(EachAxisHoldsOnlyItsOwnTags)
{
        static_assert(xstd::reading_tag<xstd::bitset_reading_tag>);
        static_assert(xstd::reading_tag<xstd::sequence_reading_tag>);
        static_assert(xstd::reading_tag<xstd::set_reading_tag>);

        static_assert(xstd::container_tag<xstd::array_container_tag>);
        static_assert(xstd::container_tag<xstd::vector_container_tag>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::container_tag<xstd::inplace_vector_container_tag>);
#endif

        // The axes are disjoint, so neither concept ever holds for the other's tags.
        static_assert(not xstd::reading_tag<xstd::array_container_tag>);
        static_assert(not xstd::container_tag<xstd::set_reading_tag>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(NeitherConceptHoldsForWhatHasNotOptedIn)
{
        static_assert(not xstd::reading_tag<int>);
        static_assert(not xstd::container_tag<int>);
        static_assert(not xstd::reading_tag<void>);
        static_assert(not xstd::container_tag<void>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
