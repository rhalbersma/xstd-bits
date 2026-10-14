//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sets.hpp>                 // for_each_set, iterators_of, sets, stops
#include <test/for_each_type.hpp>                  // for_each_type
#include <xstd/bits/bit_algorithm/bit_advance.hpp> // bit_advance
#include <xstd/bits/bit_array.hpp>                 // bit_array
#include <xstd/bits/bit_fixed_set.hpp>             // bit_fixed_set
#include <xstd/bits/bit_set.hpp>                   // bit_set
#include <xstd/bits/bit_set_view.hpp>              // bit_set_view
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                   // array
#include <concepts>                                // same_as
#include <cstddef>                                 // ptrdiff_t, size_t
#include <cstdint>                                 // uint64_t
#include <iterator>                                // advance
#include <ranges>                                  // iterator_t
#include <set>                                     // set
#include <utility>                                 // declval

BOOST_AUTO_TEST_SUITE(BitAdvance)

namespace {

template<class I>
concept advanceable = requires (I& it) { xstd::bit_advance(it, 1); };

// Every unbounded advance from each stop: forward to every reachable index, backward to every one behind.
template<class X>
[[nodiscard]] auto unbounded_disagreements(X const& s)
        -> int
{
        auto const its = test::algorithm::iterators_of(s);
        auto const n   = its.size() - 1UZ;
        auto wrong     = 0;
        for (auto const i : test::algorithm::stops(n)) {
                for (auto const j : test::algorithm::stops(n)) {
                        auto it = its[i];
                        xstd::bit_advance(it, static_cast<std::ptrdiff_t>(j) - static_cast<std::ptrdiff_t>(i));
                        wrong += static_cast<int>(it != its[j]);
                }
        }
        return wrong;
}

// Every bounded advance from each stop towards each bound at every distance, against std::ranges::advance.
template<class X>
[[nodiscard]] auto bounded_disagreements(X const& s)
        -> int
{
        auto const its = test::algorithm::iterators_of(s);
        auto const n   = its.size() - 1UZ;
        auto wrong     = 0;
        for (auto const i : test::algorithm::stops(n)) {
                for (auto const b : test::algorithm::stops(n)) {
                        for (auto const steps : {0Z, 1Z, 2Z, 9Z, 64Z, 200Z}) {
                                auto const signed_steps = b < i ? -steps : steps;
                                auto got                = its[i];
                                auto want               = its[i];
                                auto const got_rest     = xstd::bit_advance(got, signed_steps, its[b]);
                                auto const want_rest    = std::ranges::advance(want, signed_steps, its[b]);
                                wrong += static_cast<int>(got != want or got_rest != want_rest);
                        }
                }
        }
        return wrong;
}

} // namespace

// Every input of every set column of ours, ascending and descending, against stepping one key at a time.
BOOST_AUTO_TEST_CASE(AgreesWithRangesAdvanceOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_set<X>([&](X const& x) -> void { wrong += unbounded_disagreements(x) + bounded_disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A view's iterators skip the blocks they view, its owner's or lent ones, and an empty set's begin is its end.
BOOST_AUTO_TEST_CASE(AViewAdvancesOverWhatItViews)
{
        auto owner      = xstd::bit_fixed_set<200>{0UZ, 64UZ, 65UZ, 130UZ, 199UZ};
        auto const view = xstd::bit_set_view(owner);
        BOOST_CHECK_EQUAL(unbounded_disagreements(view) + bounded_disagreements(view), 0);
        auto const none = xstd::bit_set();
        BOOST_CHECK_EQUAL(unbounded_disagreements(none) + bounded_disagreements(none), 0);
        auto blocks     = std::array<std::uint64_t, 3>{0x5U, 0x0U, 0x8000'0000'0000'0001U};
        auto const lent = xstd::bit_set_view(blocks);
        BOOST_CHECK_EQUAL(unbounded_disagreements(lent) + bounded_disagreements(lent), 0);
}

// In a constant expression, and over our sets' iterators alone, answering what std::ranges::advance answers.
BOOST_AUTO_TEST_CASE(AdvancesOurSetIteratorsAlone)
{
        static_assert([] -> bool {
                auto const s = xstd::bit_fixed_set<200>{3UZ, 64UZ, 150UZ, 199UZ};
                auto it      = s.begin();
                xstd::bit_advance(it, 3);
                auto const rest = xstd::bit_advance(it, -9, s.begin());
                return *it == 3UZ and rest == -6;
        }());
        using S = xstd::bit_fixed_set<8>;
        static_assert(advanceable<std::ranges::iterator_t<S>> and advanceable<std::ranges::iterator_t<xstd::bit_set>>);
        static_assert(not advanceable<std::ranges::iterator_t<xstd::bit_array<8>>> and not advanceable<std::set<std::size_t>::iterator> and not advanceable<int*>);
        static_assert(std::same_as<decltype(xstd::bit_advance(std::declval<std::ranges::iterator_t<S>&>(), 1, std::declval<std::ranges::iterator_t<S>>())), std::ptrdiff_t>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
