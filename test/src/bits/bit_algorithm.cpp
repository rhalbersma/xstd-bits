//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm.hpp>         // bit_all_of, bit_any_of, bit_copy, bit_copy_backward, bit_count, bit_disjoint, bit_equal, bit_fill, bit_find, bit_find_last, bit_for_each, bit_includes, bit_intersects, bit_is_partitioned, bit_is_sorted, bit_lexicographical_compare_three_way, bit_mismatch, bit_move, bit_none_of, bit_partition, bit_partition_point, bit_reverse, bit_reverse_copy, bit_rotate, bit_rotate_copy, bit_set_difference, bit_set_intersection, bit_set_symmetric_difference, bit_set_union, bit_shift_left, bit_shift_right, bit_sort, bit_swap_ranges, bit_transform
#include <xstd/bits/bit_array.hpp>             // bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // bit_fixed_set
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <compare>                             // is_eq
#include <functional>                          // bit_or, logical_not
#include <iterator>                            // next
#include <ranges>                              // begin

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
        static_assert(xstd::bit_includes(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2}) and xstd::bit_disjoint(xstd::bit_fixed_set<8>{1}, xstd::bit_fixed_set<8>{2}) and xstd::bit_intersects(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2}));
        static_assert(xstd::bit_set_union(xstd::bit_fixed_set<8>{1}, xstd::bit_fixed_set<8>{2}) == xstd::bit_fixed_set<8>{1, 2} and xstd::bit_set_intersection(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2}) == xstd::bit_fixed_set<8>{2});
        static_assert(xstd::bit_set_difference(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2}) == xstd::bit_fixed_set<8>{1} and xstd::bit_set_symmetric_difference(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2, 3}) == xstd::bit_fixed_set<8>{1, 3});
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// The second batch through the same include: the writes, the searches, the walk and the comparisons.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesTheWrites)
{
        static_assert([] -> bool {
                auto a = xstd::bit_array<10>();
                auto b = xstd::bit_array<10>();
                static_cast<void>(xstd::bit_fill(a, true));
                static_cast<void>(xstd::bit_shift_left(a, 4));
                static_cast<void>(xstd::bit_copy(a, b.begin()));
                static_cast<void>(xstd::bit_move(a, b.begin()));
                static_cast<void>(xstd::bit_copy_backward(a, b.end()));
                static_cast<void>(xstd::bit_reverse_copy(a, b.begin()));
                static_cast<void>(xstd::bit_rotate_copy(a, std::ranges::begin(a), b.begin()));
                static_cast<void>(xstd::bit_swap_ranges(a, b));
                static_cast<void>(xstd::bit_transform(a, b, a.begin(), std::bit_or<>()));
                static_cast<void>(xstd::bit_transform(b, b.begin(), std::logical_not<>()));
                static_cast<void>(xstd::bit_shift_right(a, 1));
                static_cast<void>(xstd::bit_partition(a));
                auto const partitioned = xstd::bit_is_partitioned(a) and xstd::bit_partition_point(a) == xstd::bit_find(a, false);
                static_cast<void>(xstd::bit_sort(a));
                auto ones = 0;
                static_cast<void>(xstd::bit_for_each(a, [&](bool x) -> void { ones += static_cast<int>(x); }));
                return partitioned and xstd::bit_is_sorted(a) and xstd::bit_find_last(a, false).begin() == std::ranges::next(std::ranges::begin(a), 10 - ones - 1) and xstd::bit_equal(a, a) and std::is_eq(xstd::bit_lexicographical_compare_three_way(a, a));
        }());
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
