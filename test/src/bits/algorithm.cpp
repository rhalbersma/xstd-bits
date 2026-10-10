//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/algorithm.hpp>     // bit_all_of, bit_any_of, bit_count, bit_disjoint, bit_includes, bit_mismatch, bit_none_of, bit_reverse, bit_rotate
#include <xstd/bits/bit_array.hpp>     // bit_array
#include <xstd/bits/bit_fixed_set.hpp> // bit_fixed_set
#include <boost/test/unit_test.hpp>    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                      // begin, next

BOOST_AUTO_TEST_SUITE(Algorithm)

// The umbrella reaches every algorithm of both readings in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryAlgorithm)
{
        static_assert([] -> bool {
                auto a = xstd::bit_array<10>();
                a[0]   = true;
                a[8]   = true;
                xstd::bit_rotate(a, std::ranges::next(std::ranges::begin(a), 1));
                xstd::bit_reverse(a);
                auto const b = xstd::bit_array<10>();
                return xstd::bit_count(a) == 2 and a[0] and a[2] and xstd::bit_any_of(a) and not xstd::bit_all_of(a) and xstd::bit_none_of(b) and xstd::bit_mismatch(a, b).in1 == std::ranges::begin(a);
        }());
        static_assert(xstd::bit_includes(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2}) and xstd::bit_disjoint(xstd::bit_fixed_set<8>{1}, xstd::bit_fixed_set<8>{2}));
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
