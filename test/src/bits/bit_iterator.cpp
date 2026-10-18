//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_iterator.hpp>          // bit_advance, bit_distance, bit_next, bit_prev
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // bit_fixed_set
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

BOOST_AUTO_TEST_SUITE(Iterator)

// The umbrella reaches every iterator operation in one include: rank and select over a set's keys.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryIteratorOperation)
{
        static_assert([] -> bool {
                auto const s = xstd::bit_fixed_set<100>{1, 50, 99};
                auto it      = s.begin();
                xstd::ranges::bit_advance(it, 1);
                return *it == 50UZ and xstd::ranges::bit_next(s.begin(), 2) == xstd::ranges::bit_prev(s.end(), 1) and xstd::ranges::bit_distance(s.begin(), s.end()) == 3;
        }());
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
