//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                   // for_each_sequence, for_each_window, patterned, sequences, sorted
#include <test/for_each_type.hpp>                         // for_each_type
#include <xstd/bits/bit_algorithm/bit_is_partitioned.hpp> // bit_is_partitioned
#include <xstd/bits/bit_array.hpp>                        // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>            // bit_fixed_set
#include <xstd/bits/bit_span.hpp>                         // bit_span
#include <boost/test/unit_test.hpp>                       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                      // is_partitioned
#include <cstdint>                                        // uint8_t
#include <functional>                                     // identity
#include <ranges>                                         // begin, end
#include <vector>                                         // vector

BOOST_AUTO_TEST_SUITE(BitIsPartitioned)

namespace {

template<class R>
concept askable = requires (R const& r) { xstd::bit_is_partitioned(r); };

// The algorithm and std::ranges::is_partitioned(model, std::identity()) on the same bools.
template<class R>
[[nodiscard]] constexpr auto agrees(R const& r)
        -> bool
{
        auto const model = std::vector<bool>(std::ranges::begin(r), std::ranges::end(r));
        return xstd::bit_is_partitioned(r) == std::ranges::is_partitioned(model, std::identity());
}

} // namespace

// Every input of every column of ours, owners and the whole views over them, across blocks, widths and storages.
BOOST_AUTO_TEST_CASE(AgreesWithRangesIsPartitionedOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees(x)) + static_cast<int>(not agrees(xstd::bit_span(x))); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window asks its own positions alone, at every offset and length across two narrow blocks, sorted or striped.
BOOST_AUTO_TEST_CASE(AWindowAsksItsOwnPositions)
{
        auto wrong = 0;
        for (auto const& a : {test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), test::algorithm::sorted(xstd::basic_bit_array<std::uint8_t, 21>(), 9UZ)}) {
                test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(not agrees(w)); });
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, and over bools of ours alone.
BOOST_AUTO_TEST_CASE(AsksOurSequencesAlone)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>())) and agrees(test::algorithm::sorted(xstd::bit_array<70>(), 30UZ)));
        static_assert(askable<xstd::bit_array<8>> and not askable<std::vector<bool>> and not askable<xstd::bit_fixed_set<8>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
