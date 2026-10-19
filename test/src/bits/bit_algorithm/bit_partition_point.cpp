//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                    // for_each_sequence, for_each_window, sequences, sorted
#include <test/for_each_type.hpp>                          // for_each_type
#include <xstd/bits/bit_algorithm/bit_partition_point.hpp> // bit_partition_point
#include <xstd/bits/bit_array.hpp>                         // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>             // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>                 // bit_span
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                       // is_partitioned, partition_point
#include <concepts>                                        // same_as
#include <cstdint>                                         // uint8_t
#include <functional>                                      // identity
#include <ranges>                                          // begin, dangling, end
#include <vector>                                          // vector

BOOST_AUTO_TEST_SUITE(BitPartitionPoint)

namespace {

template<class R>
concept askable = requires (R const& r) { xstd::ranges::bit_partition_point(r); };

// The algorithm and std::ranges::partition_point(model, std::identity()), where the bools are partitioned at all.
template<class R>
[[nodiscard]] constexpr auto agrees(R const& r)
        -> bool
{
        auto const model = std::vector<bool>(std::ranges::begin(r), std::ranges::end(r));
        if (not std::ranges::is_partitioned(model, std::identity())) {
                return true;
        }
        return xstd::ranges::bit_partition_point(r) - std::ranges::begin(r) == std::ranges::partition_point(model, std::identity()) - model.begin();
}

} // namespace

// Every partitioned input of every column of ours, owners and whole views, across blocks, widths and storages.
BOOST_AUTO_TEST_CASE(AgreesWithRangesPartitionPointOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees(x)) + static_cast<int>(not agrees(xstd::bit_span(x))); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window of ones then zeros is partitioned wherever it is cut, at every offset and length across two narrow blocks.
BOOST_AUTO_TEST_CASE(AWindowAsksItsOwnPositions)
{
        auto a = xstd::basic_bit_array<std::uint8_t, 21>();
        for (auto const i : {0UZ, 1UZ, 2UZ, 3UZ, 4UZ, 5UZ, 6UZ, 7UZ, 8UZ, 9UZ, 10UZ}) {
                a[i] = true;
        }
        auto wrong = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(not agrees(w)); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, over bools of ours alone, and an rvalue owner's iterator dangles as in std::ranges.
BOOST_AUTO_TEST_CASE(AnswersAsRangesPartitionPointDoes)
{
        static_assert([] -> bool {
                auto a = xstd::bit_array<70>();
                for (auto const i : {0UZ, 1UZ, 2UZ}) {
                        a[i] = true;
                }
                return agrees(a) and xstd::ranges::bit_partition_point(a) == a.begin() + 3;
        }());
        static_assert(agrees(test::algorithm::sorted(xstd::bit_array<70>(), 0UZ)));
        static_assert(askable<xstd::bit_array<8>> and not askable<std::vector<bool>> and not askable<xstd::bit_fixed_set<8>>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_partition_point(xstd::bit_array<8>())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
