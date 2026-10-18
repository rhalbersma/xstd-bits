//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sets.hpp>                 // for_each_set, iterators_of, sets, stops
#include <test/for_each_type.hpp>                  // for_each_type
#include <xstd/bits/bit_array.hpp>                 // bit_array
#include <xstd/bits/bit_iterator/bit_distance.hpp> // bit_distance
#include <xstd/bits/bit_set/bit_fixed_set.hpp>     // bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>           // bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>      // bit_set_view
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                   // array
#include <concepts>                                // same_as
#include <cstddef>                                 // ptrdiff_t, size_t
#include <cstdint>                                 // uint64_t
#include <iterator>                                // distance
#include <ranges>                                  // iterator_t
#include <set>                                     // set
#include <utility>                                 // declval

BOOST_AUTO_TEST_SUITE(BitDistance)

namespace {

template<class I>
concept measurable = requires (I it) { xstd::ranges::bit_distance(it, it); };

// From each stop to each one at or after it, as std::ranges::distance counts the steps between them.
template<class X>
[[nodiscard]] auto disagreements(X const& s)
        -> int
{
        auto const its = test::algorithm::iterators_of(s);
        auto const n   = its.size() - 1UZ;
        auto wrong     = 0;
        for (auto const i : test::algorithm::stops(n)) {
                for (auto const j : test::algorithm::stops(n)) {
                        if (i <= j) {
                                wrong += static_cast<int>(xstd::ranges::bit_distance(its[i], its[j]) != std::ranges::distance(its[i], its[j]));
                        }
                }
        }
        return wrong;
}

} // namespace

// Every input of every set column of ours, ascending and descending, against stepping one key at a time.
BOOST_AUTO_TEST_CASE(AgreesWithRangesDistanceOverEverySetColumn)
{
        test::for_each_type<test::algorithm::sets>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_set<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A view's iterators count the keys it views, its owner's or lent ones, and an empty set has none between its ends.
BOOST_AUTO_TEST_CASE(AViewCountsWhatItViews)
{
        auto owner = xstd::bit_fixed_set<200>{0UZ, 64UZ, 65UZ, 130UZ, 199UZ};
        BOOST_CHECK_EQUAL(disagreements(xstd::bit_set_view(owner)), 0);
        auto const none = xstd::bit_set();
        BOOST_CHECK_EQUAL(xstd::ranges::bit_distance(none.begin(), none.end()), 0);
        auto blocks = std::array<std::uint64_t, 3>{0x5U, 0x0U, 0x8000'0000'0000'0001U};
        BOOST_CHECK_EQUAL(disagreements(xstd::bit_set_view(blocks)), 0);
}

// In a constant expression, and over our sets' iterators alone, in their difference type.
BOOST_AUTO_TEST_CASE(MeasuresOurSetIteratorsAlone)
{
        static_assert([] -> bool {
                auto const s = xstd::bit_fixed_set<200>{3UZ, 64UZ, 150UZ, 199UZ};
                return xstd::ranges::bit_distance(s.begin(), s.end()) == 4 and xstd::ranges::bit_distance(s.find(64UZ), s.find(199UZ)) == 2;
        }());
        using S = xstd::bit_fixed_set<8>;
        static_assert(measurable<std::ranges::iterator_t<S>> and not measurable<std::ranges::iterator_t<xstd::bit_array<8>>> and not measurable<std::set<std::size_t>::iterator>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_distance(std::declval<std::ranges::iterator_t<S>>(), std::declval<std::ranges::iterator_t<S>>())), std::ptrdiff_t>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
