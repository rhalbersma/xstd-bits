//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>            // for_each_sequence, for_each_window, patterned, sequences
#include <test/algorithm/sets.hpp>                 // for_each_set, sets
#include <test/for_each_type.hpp>                  // for_each_type
#include <xstd/bits/bit_algorithm/bit_find_if.hpp> // bit_find_if
#include <xstd/bits/bit_array.hpp>                 // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>     // bit_fixed_set
#include <xstd/bits/bit_set/bit_set_view.hpp>      // bit_set_view
#include <xstd/bits/bit_span/bit_span.hpp>         // bit_span
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                               // find_if
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint8_t
#include <ranges>                                  // begin, dangling, end
#include <set>                                     // set
#include <vector>                                  // vector

BOOST_AUTO_TEST_SUITE(BitFindIf)

namespace {

template<class R, class Pred>
concept findable_if = requires (R const& r, Pred pred) { xstd::ranges::bit_find_if(r, pred); };

// The algorithm and std::ranges::find_if over r with pred: the same iterator.
template<class R, class Pred>
[[nodiscard]] constexpr auto agrees(R const& r, Pred pred)
        -> bool
{
        return xstd::ranges::bit_find_if(r, pred) == std::ranges::find_if(r, pred);
}

// Keys picked by residue, by threshold and none at all, so the first hit falls early, late, mid-block and nowhere.
template<class R>
[[nodiscard]] constexpr auto agrees_on_keys(R const& r)
        -> bool
{
        return agrees(r, [](std::size_t k) -> bool { return k % 3UZ == 2UZ; }) and agrees(r, [](std::size_t k) -> bool { return k >= 64UZ; }) and agrees(r, [](std::size_t k) -> bool { return k < 2UZ; }) and agrees(r, [](std::size_t) -> bool { return false; }) and agrees(r, [](std::size_t) -> bool { return true; });
}

// Each of the four predicates a bool has.
template<class R>
[[nodiscard]] constexpr auto agrees_on_bools(R const& r)
        -> bool
{
        return agrees(r, [](bool b) -> bool { return b; }) and agrees(r, [](bool b) -> bool { return not b; }) and agrees(r, [](bool) -> bool { return true; }) and agrees(r, [](bool) -> bool { return false; });
}

} // namespace

// Every input of every set column of ours, ascending and descending, the first key in each one's own order.
BOOST_AUTO_TEST_CASE(AgreesWithRangesFindIfOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_set<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees_on_keys(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// Every input of every sequence column of ours, owners and the whole views over them.
BOOST_AUTO_TEST_CASE(AgreesWithRangesFindIfOverEverySequenceColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += static_cast<int>(not agrees_on_bools(x)) + static_cast<int>(not agrees_on_bools(xstd::bit_span(x))); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window searches its own positions alone, and a set view the keys it views.
BOOST_AUTO_TEST_CASE(AWindowAndASetViewSearchWhatTheyView)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += static_cast<int>(not agrees_on_bools(w)); });
        BOOST_CHECK_EQUAL(wrong, 0);
        auto owner = xstd::bit_fixed_set<130>{0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ};
        BOOST_CHECK(agrees_on_keys(xstd::bit_set_view(owner)));
}

// Over a set, pred is applied to each key before the answer and to the answer, and the walk stops there.
BOOST_AUTO_TEST_CASE(OverASetItStopsAtTheFirstKeyAccepted)
{
        auto const s = xstd::bit_fixed_set<130>{0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ};
        auto calls   = 0UZ;
        auto const i = xstd::ranges::bit_find_if(s, [&](std::size_t k) -> bool { ++calls; return k > 63UZ; });
        BOOST_CHECK_EQUAL(*i, 64UZ);
        BOOST_CHECK_EQUAL(calls, 4UZ);
        calls = 0UZ;
        BOOST_CHECK(xstd::ranges::bit_find_if(s, [&](std::size_t) -> bool { ++calls; return false; }) == s.end());
        BOOST_CHECK_EQUAL(calls, s.size());
}

// Over a sequence, an equality-preserving pred is asked about each bool once, and about the one element of a singleton.
BOOST_AUTO_TEST_CASE(OverASequenceItAsksAboutEachBoolOnce)
{
        auto const a = test::algorithm::patterned(xstd::bit_array<130>());
        auto calls   = 0UZ;
        auto const i = xstd::ranges::bit_find_if(a, [&](bool b) -> bool { ++calls; return not b; });
        BOOST_CHECK(i == std::ranges::find_if(a, [](bool b) -> bool { return not b; }));
        BOOST_CHECK_EQUAL(calls, 2UZ);
        auto const one = xstd::bit_array<1>();
        calls          = 0UZ;
        BOOST_CHECK(xstd::ranges::bit_find_if(one, [&](bool b) -> bool { ++calls; return not b; }) == one.begin());
        BOOST_CHECK_EQUAL(calls, 1UZ);
}

// A pred that answers both bools alike finds the first element or none, asked twice, whatever the bits hold.
BOOST_AUTO_TEST_CASE(OverASequenceAConstantPredFindsTheFirstOrNone)
{
        auto const a = test::algorithm::patterned(xstd::bit_array<130>());
        auto calls   = 0UZ;
        BOOST_CHECK(xstd::ranges::bit_find_if(a, [&](bool) -> bool { ++calls; return true; }) == a.begin());
        BOOST_CHECK(xstd::ranges::bit_find_if(a, [&](bool) -> bool { ++calls; return false; }) == a.end());
        BOOST_CHECK_EQUAL(calls, 4UZ);
}

// pred takes each element by value, so a reference to one is refused, as are the standard containers.
BOOST_AUTO_TEST_CASE(AnswersAsRangesFindIfDoes)
{
        using S = xstd::bit_fixed_set<8>;
        using A = xstd::bit_array<8>;
        static_assert(agrees_on_bools(test::algorithm::patterned(xstd::bit_array<70>())));
        static_assert(agrees_on_keys(xstd::bit_fixed_set<70>{2UZ, 5UZ, 64UZ, 69UZ}));
        static_assert(findable_if<S, decltype([](std::size_t) -> bool { return true; })> and findable_if<A, decltype([](bool) -> bool { return true; })>);
        static_assert(not findable_if<S, decltype([](std::size_t&) -> bool { return true; })> and not findable_if<A, decltype([](bool&) -> bool { return true; })>);
        static_assert(not findable_if<std::vector<bool>, decltype([](bool) -> bool { return true; })> and not findable_if<std::set<std::size_t>, decltype([](std::size_t) -> bool { return true; })>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_find_if(S(), [](std::size_t) -> bool { return true; })), std::ranges::dangling>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_find_if(A(), [](bool) -> bool { return true; })), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
