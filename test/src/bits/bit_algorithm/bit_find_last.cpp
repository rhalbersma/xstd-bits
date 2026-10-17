//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>              // for_each_sequence, for_each_window, patterned, sequences, sorted
#include <test/for_each_type.hpp>                    // for_each_type
#include <xstd/bits/bit_algorithm/bit_find_last.hpp> // bit_find_last
#include <xstd/bits/bit_array.hpp>                   // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>       // bit_fixed_set
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                 // all_of, find_last
#include <array>                                     // array
#include <concepts>                                  // same_as
#include <cstdint>                                   // uint8_t
#include <ranges>                                    // begin, dangling, end
#include <vector>                                    // vector

BOOST_AUTO_TEST_SUITE(BitFindLast)

namespace {

template<class R>
concept findable = requires (R const& r) { xstd::bit_find_last(r, true); };

// The algorithm and std::ranges::find_last on the same bools, for either value: where each subrange starts and ends.
template<class R>
[[nodiscard]] constexpr auto agrees(R const& r)
        -> bool
{
        auto const model = std::vector<bool>(std::ranges::begin(r), std::ranges::end(r));
        return std::ranges::all_of(std::array{false, true}, [&](bool value) -> bool {
                auto const got  = xstd::bit_find_last(r, value);
                auto const want = std::ranges::find_last(model, value);
                return got.begin() - std::ranges::begin(r) == want.begin() - model.begin() and got.end() == std::ranges::end(r);
        });
}

} // namespace

// Every input of every column of ours, owners and the whole views over them, across blocks, widths and storages.
BOOST_AUTO_TEST_CASE(AgreesWithRangesFindLastOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees(x)) + static_cast<int>(not agrees(xstd::bit_span(x))); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window searches its own positions alone, from its top, at every offset and length across two narrow blocks.
BOOST_AUTO_TEST_CASE(AWindowSearchesItsOwnPositions)
{
        auto wrong = 0;
        for (auto const& a : {test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), test::algorithm::sorted(xstd::basic_bit_array<std::uint8_t, 21>(), 9UZ)}) {
                test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(not agrees(w)); });
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, over bools of ours alone, and an rvalue owner's subrange dangles as in std::ranges.
BOOST_AUTO_TEST_CASE(AnswersAsRangesFindLastDoes)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>())));
        static_assert(findable<xstd::bit_array<8>> and not findable<std::vector<bool>> and not findable<xstd::bit_fixed_set<8>>);
        static_assert(std::same_as<decltype(xstd::bit_find_last(xstd::bit_array<8>(), true)), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
