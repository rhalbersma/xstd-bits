//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>              // bools_of, for_each_sequence, patterned, sequences, writes_as_model
#include <test/for_each_type.hpp>                    // for_each_type
#include <xstd/bits/bit_algorithm/bit_partition.hpp> // bit_partition
#include <xstd/bits/bit_array.hpp>                   // bit_array, basic_bit_array
#include <xstd/bits/bit_fixed_set.hpp>               // bit_fixed_set
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                 // partition
#include <concepts>                                  // same_as
#include <cstdint>                                   // uint8_t
#include <functional>                                // identity
#include <ranges>                                    // begin, dangling, end
#include <utility>                                   // declval, forward
#include <vector>                                    // vector

BOOST_AUTO_TEST_SUITE(BitPartition)

namespace {

template<class R>
concept permutable = requires (R&& r) { xstd::bit_partition(std::forward<R>(r)); };

// The algorithm on x, as owner or whole view, and std::ranges::partition on its bools: ones, then zeros from the split.
template<class X>
[[nodiscard]] constexpr auto agrees(X x, bool through_view)
        -> bool
{
        auto model        = test::algorithm::bools_of(x);
        auto const want   = std::ranges::partition(model, std::identity());
        auto const splits = [&](auto&& r) -> bool {
                auto const [first, last] = xstd::bit_partition(r);
                return first - std::ranges::begin(r) == want.begin() - model.begin() and last == std::ranges::end(r);
        };
        auto const split = through_view ? splits(xstd::bit_span(x)) : splits(x);
        return split and test::algorithm::bools_of(x) == model;
}

} // namespace

// Every input of every column of ours, as an owner and through a whole view, across blocks, widths and storages.
BOOST_AUTO_TEST_CASE(AgreesWithRangesPartitionOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees(x, false)) + static_cast<int>(not agrees(x, true)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window permutes its own positions and leaves every other one as it was, at every offset and length.
BOOST_AUTO_TEST_CASE(AWindowPermutesItsOwnPositionsAlone)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        BOOST_CHECK_EQUAL(test::algorithm::writes_as_model(a, [](auto w) -> void { static_cast<void>(xstd::bit_partition(w)); }, [](auto model) -> void { static_cast<void>(std::ranges::partition(model, std::identity())); }), 0);
}

// In a constant expression, and over writable bools of ours alone: never a const owner, a set or std::vector<bool>.
BOOST_AUTO_TEST_CASE(PermutesOurWritableSequencesAlone)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>()), false) and agrees(test::algorithm::patterned(xstd::bit_array<70>()), true));
        static_assert(permutable<xstd::bit_array<8>&> and permutable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()).subspan(1UZ, 3UZ))>);
        static_assert(not permutable<xstd::bit_array<8> const&> and not permutable<std::vector<bool>&> and not permutable<xstd::bit_fixed_set<8>&>);
        static_assert(std::same_as<decltype(xstd::bit_partition(xstd::bit_array<8>())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
