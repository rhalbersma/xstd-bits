//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>             // for_each_sequence, for_each_window, patterned, sequences
#include <test/algorithm/sets.hpp>                  // for_each_set, sets
#include <test/for_each_type.hpp>                   // for_each_type
#include <xstd/bits/bit_algorithm/bit_for_each.hpp> // bit_for_each
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>      // bit_fixed_set
#include <xstd/bits/bit_set/bit_set_view.hpp>       // bit_set_view
#include <xstd/bits/bit_span/bit_span.hpp>          // bit_span
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                // for_each, for_each_result
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t
#include <ranges>                                   // dangling, end, range_value_t, reverse
#include <set>                                      // set
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitForEach)

namespace {

template<class R, class F>
concept walks = requires (R const& r, F f) { xstd::bit_for_each(r, f); };

// What a walk saw, in the order it saw it, carried back in the functor the algorithm returns.
template<class T>
struct collector
{
        std::vector<T> seen;

        constexpr auto operator()(T x)
                -> void
        {
                seen.push_back(x);
        }
};

// The algorithm and std::ranges::for_each over r: the elements in order, the end, and the functor handed back.
template<class R>
[[nodiscard]] constexpr auto agrees(R&& r)
        -> bool
{
        using T         = std::ranges::range_value_t<R>;
        auto const want = std::ranges::for_each(r, collector<T>());
        auto const got  = xstd::bit_for_each(r, collector<T>());
        return got.in == std::ranges::end(r) and got.fun.seen == want.fun.seen;
}

// Forward and through std::views::reverse, which walks the other way.
template<class R>
[[nodiscard]] constexpr auto disagreements(R const& r)
        -> int
{
        return static_cast<int>(not agrees(r)) + static_cast<int>(not agrees(std::views::reverse(r)));
}

// Functors overloaded on the value category, to see which one the walk calls.
template<class T>
struct probe
{
        bool* took_a_reference;

        auto operator()(T&&) const -> void {}

        // Never called is exactly what is under test, so say so rather than let -Wunused-member-function say it.
        [[maybe_unused]] auto operator()(T&) const
                -> void
        {
                *took_a_reference = true;
        }
};

} // namespace

// Every input of every set column of ours, ascending and descending, forward and reversed.
BOOST_AUTO_TEST_CASE(AgreesWithRangesForEachOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_set<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// Every input of every sequence column of ours, owners and the whole views over them, forward and reversed.
BOOST_AUTO_TEST_CASE(AgreesWithRangesForEachOverEverySequenceColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window walks its own positions alone, either way, and a set view the keys it views, reversed by value.
BOOST_AUTO_TEST_CASE(AWindowAndASetViewWalkWhatTheyView)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
        auto owner = xstd::bit_fixed_set<130>{0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ};
        BOOST_CHECK(agrees(xstd::bit_set_view(owner)) and agrees(std::views::reverse(xstd::bit_set_view(owner))));
}

// A functor returning bool is called on every element, as std::ranges::for_each calls it, its answer ignored.
BOOST_AUTO_TEST_CASE(ABoolReturningFunctorDoesNotStopTheWalk)
{
        auto const s = xstd::bit_fixed_set<130>{0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 129UZ};
        auto calls   = 0UZ;
        static_cast<void>(xstd::bit_for_each(s, [&](std::size_t) -> bool { ++calls; return false; }));
        BOOST_CHECK_EQUAL(calls, s.size());
}

// The functor takes each element by value: a reference to one is refused, and the prvalue overload is the one called.
BOOST_AUTO_TEST_CASE(TheFunctorTakesAValue)
{
        using S = xstd::bit_fixed_set<8>;
        using A = xstd::bit_array<8>;
        static_assert(walks<S, decltype([](std::size_t) -> void {})> and walks<S, decltype([](auto const&) -> void {})> and walks<S, void (*)(std::size_t)>);
        static_assert(not walks<S, decltype([](std::size_t&) -> void {})> and not walks<S, decltype([](auto&) -> void {})>);
        static_assert(walks<A, decltype([](bool) -> void {})> and not walks<A, decltype([](bool&) -> void {})>);
        static_assert(not walks<std::vector<bool>, decltype([](bool) -> void {})> and not walks<std::set<std::size_t>, decltype([](std::size_t) -> void {})>);
        auto took_a_reference = false;
        auto const s          = S{3UZ};
        auto const a          = test::algorithm::patterned(A());
        static_cast<void>(xstd::bit_for_each(s, probe<std::size_t>{&took_a_reference}));
        static_cast<void>(xstd::bit_for_each(std::views::reverse(s), probe<std::size_t>{&took_a_reference}));
        static_cast<void>(xstd::bit_for_each(a, probe<bool>{&took_a_reference}));
        static_cast<void>(xstd::bit_for_each(std::views::reverse(a), probe<bool>{&took_a_reference}));
        BOOST_CHECK(not took_a_reference);
}

// In a constant expression, and with what std::ranges::for_each answers, an rvalue owner's iterator dangling.
BOOST_AUTO_TEST_CASE(AnswersAsRangesForEachDoes)
{
        static_assert(disagreements(test::algorithm::patterned(xstd::bit_array<70>())) == 0);
        static_assert(disagreements(xstd::bit_fixed_set<70>{0UZ, 3UZ, 64UZ, 69UZ}) == 0);
        static_assert(std::same_as<decltype(xstd::bit_for_each(xstd::bit_array<8>(), collector<bool>())), std::ranges::for_each_result<std::ranges::dangling, collector<bool>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
