//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/ownership.hpp> // reading, storage, owns
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Ownership)

BOOST_AUTO_TEST_CASE(OwningIsOneOfTwoAnswers)
{
        static_assert(owns(xstd::bits::detail::storage::owned));
        static_assert(not owns(xstd::bits::detail::storage::borrowed));
        BOOST_CHECK(xstd::bits::detail::storage::owned != xstd::bits::detail::storage::borrowed);
}

// A bitset is a hybrid of the other two rather than a refinement, so the three stand apart with none nested inside another.
BOOST_AUTO_TEST_CASE(TheReadingsAreThreeAndDistinct)
{
        static_assert(xstd::bits::detail::reading::set != xstd::bits::detail::reading::sequence);
        static_assert(xstd::bits::detail::reading::sequence != xstd::bits::detail::reading::bitset);
        static_assert(xstd::bits::detail::reading::bitset != xstd::bits::detail::reading::set);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
