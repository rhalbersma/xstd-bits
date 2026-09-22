//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/ownership.hpp> // reading, storage, owns
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Ownership)

BOOST_AUTO_TEST_CASE(OwningIsOneOfTwoAnswers)
{
        static_assert(owns(xstd::storage::owned));
        static_assert(not owns(xstd::storage::borrowed));
        BOOST_CHECK(xstd::storage::owned != xstd::storage::borrowed);
}

// A bitset is a hybrid of the other two rather than a refinement, so the three stand apart with none nested inside another.
BOOST_AUTO_TEST_CASE(TheReadingsAreThreeAndDistinct)
{
        static_assert(xstd::reading::set != xstd::reading::sequence);
        static_assert(xstd::reading::sequence != xstd::reading::bitset);
        static_assert(xstd::reading::bitset != xstd::reading::set);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
