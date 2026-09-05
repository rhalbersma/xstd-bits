//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/set/ordering.hpp>                  // ordering_agrees_with_std_set
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // set_find, set_compare over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // set_find, set_compare over std::bitset
#include <xstd/bits/ranges/set_view.hpp>          // set_view, set_range
#include <bitset>                                 // bitset
#include <concepts>                               // same_as
#include <ranges>                                 // bidirectional_range
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(Ranges)
BOOST_AUTO_TEST_SUITE(SetView)

namespace {

// One static extent of each library, and the dynamic one.
using ViewedTypes = std::tuple<std::bitset<8>, xstd::bitset<8>, boost::dynamic_bitset<>>;

// dynamic_bitset alone needs its width at construction; the others carry theirs in the type.
template<class T>
auto eight_bits_with_three_set() -> T
{
        auto bits = T();
        if constexpr (std::same_as<T, boost::dynamic_bitset<>>) {
                bits.resize(8);
        }
        bits.set(3);
        return bits;
}

}  // namespace

// The types a set_view exists for: those holding a set of positions without offering it, which bit_static_set already does.
BOOST_AUTO_TEST_CASE(TheViewedTypesAreTheOnesHoldingASetWithoutOfferingIt)
{
        static_assert(xstd::ranges::set_range<std::bitset<8>>);
        static_assert(xstd::ranges::set_range<xstd::bitset<8>>);
        static_assert(xstd::ranges::set_range<boost::dynamic_bitset<>>);

        // None of them is a range on its own; that is what the view supplies.
        static_assert(not std::ranges::range<std::bitset<8>>);
        static_assert(not std::ranges::range<xstd::bitset<8>>);

        static_assert(std::ranges::bidirectional_range<xstd::set_view<std::bitset<8>>>);
        static_assert(std::ranges::bidirectional_range<xstd::set_view<xstd::bitset<8>>>);
        static_assert(std::ranges::bidirectional_range<xstd::set_view<boost::dynamic_bitset<>>>);
}

// Asking is total whatever the extent, exactly as [set] has it. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryExtentAnswersForPositionsPastItsWidth, T, ViewedTypes)
{
        auto bits = eight_bits_with_three_set<T>();
        auto const v = xstd::set_view(bits);
        auto const width = v.max_size();

        // Both ways round, so that find and lower_bound are each seen taking either arm.
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.count(3), 1);
        // find is the subject here, which is the one call readability-container-contains would remove.
        BOOST_CHECK(v.find(3) != v.end());  // NOLINT(readability-container-contains)
        BOOST_CHECK(v.lower_bound(3) == v.find(3));

        BOOST_CHECK(not v.contains(99));
        BOOST_CHECK_EQUAL(v.count(99), 0);
        BOOST_CHECK(v.find(99) == v.end());  // NOLINT(readability-container-contains)
        BOOST_CHECK(v.lower_bound(99) == v.end());

        // Asked from inside the width, where the scan actually runs: nothing is above 3 here.
        BOOST_CHECK(v.upper_bound(3) == v.end());
        BOOST_CHECK(v.upper_bound(99) == v.end());

        // Erasing what is not there is std::set::erase's no-op returning zero. [design.md#asking-is-total]
        BOOST_CHECK_EQUAL(v.erase(99), 0);
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.size(), 1);
        BOOST_CHECK_EQUAL(v.max_size(), width);
}

// [set] gives insert no way to fail, so a dynamic extent grows rather than asserting. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE(ADynamicExtentGrowsToHoldAPositionPastItsCurrentSize)
{
        auto bits = boost::dynamic_bitset<>(8);
        bits.set(3);
        auto const v = xstd::set_view(bits);

        auto const [ where, inserted ] = v.insert(99);
        BOOST_CHECK(inserted);
        BOOST_CHECK(where != v.end());
        BOOST_CHECK(v.contains(99));
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.size(), 2);
        BOOST_CHECK_EQUAL(v.max_size(), 100);
        BOOST_CHECK_EQUAL(bits.size(), 100);

        // And within the size it is the plain write, growing nothing.
        v.insert(5);
        BOOST_CHECK(v.contains(5));
        BOOST_CHECK_EQUAL(v.max_size(), 100);

        v.erase(99);
        BOOST_CHECK(not v.contains(99));
        BOOST_CHECK_EQUAL(v.max_size(), 100);
}

// The set ordering against std::set rather than a restatement of it, for every viewed type including the one whose own <=> disagrees.
BOOST_AUTO_TEST_CASE(EveryViewedTypeOrdersLikeAStdSet)
{
        test::set::ordering_agrees_with_std_set<std::bitset<8>>();
        test::set::ordering_agrees_with_std_set<xstd::bitset<8>>();
        test::set::ordering_agrees_with_std_set<boost::dynamic_bitset<>>();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
