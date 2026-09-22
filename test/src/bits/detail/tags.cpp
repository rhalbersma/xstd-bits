//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/tags.hpp> // array_container_tag, container_tag, inplace_vector_container_tag, vector_container_tag
#include <test/inplace_vector.hpp>   // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <boost/test/unit_test.hpp>  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Tags)

BOOST_AUTO_TEST_CASE(EveryStorageTheLibraryShipsHasOptedIn)
{
        static_assert(xstd::container_tag<xstd::array_container_tag>);
        static_assert(xstd::container_tag<xstd::vector_container_tag>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::container_tag<xstd::inplace_vector_container_tag>);
#endif
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(TheConceptDoesNotHoldForWhatHasNotOptedIn)
{
        static_assert(not xstd::container_tag<int>);
        static_assert(not xstd::container_tag<void>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
