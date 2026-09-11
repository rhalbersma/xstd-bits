//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/ordering.hpp>                  // ordering_agrees_with_std_set
#include <xstd/bits/bit_set_view.hpp>             // bit_set_view
#include <xstd/bits/bit_static_set.hpp>           // bit_static_set
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/detail/block_array.hpp>       // block_array
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_traits over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bit_traits over std::bitset
#include <xstd/bits/ownership.hpp>                // ownership
#include <xstd/bits/set_adaptor.hpp>              // set_adaptor
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <bitset>                                 // bitset
#include <concepts>                               // derived_from, same_as
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t
#include <functional>                             // hash
#include <range/v3/view/set_algorithm.hpp>        // set_union
#include <ranges>                                 // bidirectional_range, borrowed_range, range, view
#include <set>                                    // set
#include <tuple>                                  // tuple
#include <utility>                                // declval

BOOST_AUTO_TEST_SUITE(BitSetView)

namespace {

// One static extent of each library, and the dynamic one.
using ViewedTypes = std::tuple<std::bitset<8>, xstd::bitset<8>, boost::dynamic_bitset<>>;

// dynamic_bitset alone needs its width at construction; the others carry theirs in the type.
template<class T>
auto eight_bits_with_three_set()
        -> T
{
        auto bits = T();
        if constexpr (std::same_as<T, boost::dynamic_bitset<>>) {
                bits.resize(8);
        }
        bits.set(3);
        return bits;
}

template<class T>
using view_of = decltype(xstd::bit_set_view(std::declval<T&>()));

}  // namespace

// The view is the referring adaptor under another name, and over an owner it refers into the storage the owner wraps. [design.md#the-views-are-the-adaptors]
BOOST_AUTO_TEST_CASE(TheViewIsTheReferringAdaptor)
{
        static_assert(std::derived_from<xstd::bit_set_view<std::bitset<8>>, xstd::set_adaptor<std::bitset<8>, xstd::ownership::refers>>);
        static_assert(std::same_as<view_of<std::bitset<8>>,          xstd::bit_set_view<std::bitset<8>>>);
        static_assert(std::same_as<view_of<std::bitset<8> const>,    xstd::bit_set_view<std::bitset<8> const>>);
        static_assert(std::same_as<view_of<boost::dynamic_bitset<>>, xstd::bit_set_view<boost::dynamic_bitset<>>>);

        static_assert(std::same_as<view_of<xstd::bitset<8>>,         xstd::bit_set_view<xstd::detail::bits::block_array<std::size_t, 8>>>);
        static_assert(std::same_as<view_of<xstd::bitset<8> const>,   xstd::bit_set_view<xstd::detail::bits::block_array<std::size_t, 8> const>>);
        static_assert(std::same_as<view_of<xstd::bit_static_set<8>>, xstd::bit_set_view<xstd::detail::bits::block_array<std::size_t, 8>>>);
}

// The types a bit_set_view exists for: those holding a set of positions without offering it, which bit_static_set already does.
BOOST_AUTO_TEST_CASE(TheViewedTypesAreTheOnesHoldingASetWithoutOfferingIt)
{
        // None of them is a range on its own; that is what the view supplies, and it is a view in std::ranges' sense, borrowed like span. [design.md#views-follow-their-precedent]
        static_assert(not std::ranges::range<std::bitset<8>>);
        static_assert(not std::ranges::range<xstd::bitset<8>>);

        static_assert(std::ranges::bidirectional_range<view_of<std::bitset<8>>>);
        static_assert(std::ranges::bidirectional_range<view_of<xstd::bitset<8>>>);
        static_assert(std::ranges::bidirectional_range<view_of<boost::dynamic_bitset<>>>);

        static_assert(std::ranges::view<view_of<std::bitset<8>>>);
        static_assert(std::ranges::borrowed_range<view_of<std::bitset<8>>>);
        static_assert(not std::ranges::view<xstd::bit_static_set<8>>);
}

// The view hashes as std::string_view does: the set it presents, so the owner's set reading of the same bits hashes the same, and at a run-time width the width is capacity there too. [design.md#the-hashing-invariant]
BOOST_AUTO_TEST_CASE(TheViewHashesAsAValue)
{
        auto bits = xstd::bitset<8>("00101010");
        auto const owned = xstd::bit_static_set<8>({ 1, 3, 5 });
        BOOST_CHECK_EQUAL(std::hash<view_of<xstd::bitset<8>>>()(xstd::bit_set_view(bits)), std::hash<xstd::bit_static_set<8>>()(owned));

        auto narrow = boost::dynamic_bitset<>(8);
        auto wide   = boost::dynamic_bitset<>(64);
        for (auto const i : { 1UZ, 3UZ, 5UZ }) {
                narrow.set(i);
                wide.set(i);
        }
        BOOST_CHECK_EQUAL(std::hash<view_of<boost::dynamic_bitset<>>>()(xstd::bit_set_view(narrow)), std::hash<view_of<boost::dynamic_bitset<>>>()(xstd::bit_set_view(wide)));
}

// Asking is total whatever the extent, exactly as [set] has it. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE_TEMPLATE(EveryExtentAnswersForPositionsPastItsWidth, T, ViewedTypes)
{
        auto bits = eight_bits_with_three_set<T>();
        auto const v = xstd::bit_set_view(bits);

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
}

// [set] gives insert no way to fail, so a dynamic extent grows rather than asserting. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE(ADynamicExtentGrowsToHoldAPositionPastItsCurrentSize)
{
        auto bits = boost::dynamic_bitset<>(8);
        bits.set(3);
        auto const v = xstd::bit_set_view(bits);

        auto const [ where, inserted ] = v.insert(99);
        BOOST_CHECK(inserted);
        BOOST_CHECK(where != v.end());
        BOOST_CHECK(v.contains(99));
        BOOST_CHECK(v.contains(3));
        BOOST_CHECK_EQUAL(v.size(), 2);
        BOOST_CHECK_EQUAL(bits.size(), 100);

        // And within the size it is the plain write, growing nothing.
        v.insert(5);
        BOOST_CHECK(v.contains(5));
        BOOST_CHECK_EQUAL(bits.size(), 100);

        v.erase(99);
        BOOST_CHECK(not v.contains(99));
        BOOST_CHECK_EQUAL(bits.size(), 100);
}

// The set ordering against std::set rather than a restatement of it, for every viewed type including the one whose own <=> disagrees.
BOOST_AUTO_TEST_CASE(EveryViewedTypeOrdersLikeAStdSet)
{
        test::set::ordering_agrees_with_std_set<std::bitset<8>>();
        test::set::ordering_agrees_with_std_set<xstd::bitset<8>>();
        test::set::ordering_agrees_with_std_set<boost::dynamic_bitset<>>();
}

// One block cannot reach the arm the word-parallel comparison exists for. An eight-bit block makes position 8
// the second block's first, so a universe of nine spans two blocks and one of eighteen spans three, and the
// pair {0} against {8} -- the one an earlier comparator got backwards -- is inside the first sweep.
// [design.md#the-ordering-primitive]
BOOST_AUTO_TEST_CASE(TheOrderingSpansBlocksAndNotJustPositions)
{
        test::set::ordering_agrees_with_std_set<xstd::basic_bitset<std::uint8_t, 9>>(9);

        // Three blocks, where "anything above" has to look past the next block as well as into it. 2^18 squared
        // is not a sweep, so this one samples. [design.md#counted-not-asserted]
        test::set::ordering_agrees_with_std_set_sampled<xstd::basic_bitset<std::uint8_t, 18>>(18UZ, 20000UZ);
        test::set::ordering_agrees_with_std_set_sampled<boost::dynamic_bitset<std::uint8_t>>(18UZ, 20000UZ);
}

// A view of keys composes with the lazy set algebra the way an owner does; the block-wise operators are the owner's shortcut around it.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheLazySetAlgebraRunsOverTheView, T, ViewedTypes)
{
        auto x = eight_bits_with_three_set<T>();
        auto y = eight_bits_with_three_set<T>();

        // Named, because range-v3's own viewable_range predates P2415 and takes a view only by lvalue or by its own view marker.
        auto const xv = xstd::bit_set_view(x);
        auto const yv = xstd::bit_set_view(y);
        xv.insert({ 1, 5 });
        yv.insert({ 5, 7 });

        auto merged = std::set<std::size_t>();
        for (std::size_t const k : ::ranges::views::set_union(xv, yv)) {
                merged.insert(k);
        }
        BOOST_CHECK((merged == std::set<std::size_t>{ 1, 3, 5, 7 }));
}

BOOST_AUTO_TEST_SUITE_END()
