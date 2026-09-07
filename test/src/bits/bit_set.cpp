//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/set/concepts.hpp>        // bit_set
#include <xstd/bits/set_adaptor.hpp>  // set_adaptor
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/block_sequence.hpp> // block_vector
#include <xstd/bits/ownership.hpp>      // ownership
#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <algorithm>                    // equal
#include <concepts>                     // same_as
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t
#include <functional>                   // hash
#include <memory>                       // allocator
#include <ranges>                       // iota, to
#include <set>                          // set

BOOST_AUTO_TEST_SUITE(BitSet)

using T = xstd::basic_bit_set<std::uint8_t>;

// The flagship: the set reading over a heap of blocks, an alias and nothing more. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheDynamicSetIsTheSetAdaptorOverAHeapOfBlocks)
{
        static_assert(std::same_as<T, xstd::set_adaptor<xstd::block_vector<std::uint8_t>, xstd::ownership::owns>>);
        static_assert(std::same_as<xstd::basic_bit_set<std::uint8_t, std::allocator<std::uint8_t>>, T>);
        static_assert(test::set::bit_set<T>);
}

// A key past the width grows the width: insert is the one operation a dynamic set cannot refuse. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE(InsertingPastTheWidthGrowsIt)
{
        auto s = T();
        BOOST_CHECK(s.empty());
        BOOST_CHECK_EQUAL(s.max_size(), xstd::block_vector<std::uint8_t>().max_size());

        auto const [ where, inserted ] = s.insert(100);
        BOOST_CHECK(inserted);
        BOOST_CHECK(*where == 100UZ);
        BOOST_CHECK(s.contains(100));
        BOOST_CHECK_EQUAL(s.size(), 1UZ);

        // Erasing never shrinks the width, and asking below or above it stays total.
        BOOST_CHECK_EQUAL(s.erase(100), 1UZ);
        BOOST_CHECK(not s.contains(100));
        BOOST_CHECK(not s.contains(1000));
        BOOST_CHECK(s.find(1000) == s.end());  // NOLINT(readability-container-contains)
}

// Built from a range as std::set is, iterated as std::set is, and ordered as std::set is.
BOOST_AUTO_TEST_CASE(ItIsBuiltAndOrderedLikeAStdSet)
{
        auto const s = std::views::iota(0UZ, 300UZ) | std::views::filter([](auto i) { return i % 7 == 0; }) | std::ranges::to<T>();
        auto const k = std::views::iota(0UZ, 300UZ) | std::views::filter([](auto i) { return i % 7 == 0; }) | std::ranges::to<std::set<std::size_t>>();
        BOOST_CHECK(std::ranges::equal(s, k));

        auto t = s;
        t.insert(1);
        BOOST_CHECK(t < s);
        BOOST_CHECK(s.is_subset_of(t));
        BOOST_CHECK(t.intersects(s));

        // The view over it refers into the block_vector, as over every owner. [design.md#views-over-owners]
        auto const v = xstd::bit_set_view(t);
        BOOST_CHECK(*v.begin() == 0UZ);
        BOOST_CHECK_EQUAL(v.size(), t.size());
}

// The width is capacity, never value: two sets holding the same positions agree on everything std::set answers, whatever their storages' widths. [design.md#width-is-capacity]
BOOST_AUTO_TEST_CASE(TheWidthIsCapacityNotValue)
{
        auto const narrow = T({ 1, 3 });
        auto wide = T({ 1, 3 });
        wide.insert(100);
        wide.erase(100);
        auto const digest = std::hash<T>();
        BOOST_CHECK(narrow == wide);
        BOOST_CHECK((narrow <=> wide) == 0);
        BOOST_CHECK_EQUAL(digest(narrow), digest(wide));
        BOOST_CHECK(digest(narrow) != digest(T({ 1 })));
        BOOST_CHECK(narrow.is_subset_of(wide) and wide.is_subset_of(narrow));
        BOOST_CHECK(not narrow.is_proper_subset_of(wide));
        BOOST_CHECK(narrow.intersects(wide));
}

// The compound operators and predicates at two widths that differ, either way round, against the answers over the elements. [design.md#width-is-capacity]
BOOST_AUTO_TEST_CASE(TheSetOperationsIgnoreTheWidth)
{
        auto const a = T({ 1, 3, 200 });
        auto const b = T({ 3, 5 });
        BOOST_CHECK((a | b) == T({ 1, 3, 5, 200 }));
        BOOST_CHECK((b | a) == T({ 1, 3, 5, 200 }));
        BOOST_CHECK((a & b) == T({ 3 }));
        BOOST_CHECK((b & a) == T({ 3 }));
        BOOST_CHECK((a ^ b) == T({ 1, 5, 200 }));
        BOOST_CHECK((b ^ a) == T({ 1, 5, 200 }));
        BOOST_CHECK((a - b) == T({ 1, 200 }));
        BOOST_CHECK((b - a) == T({ 5 }));
        BOOST_CHECK(a < b);
        BOOST_CHECK(b.is_proper_subset_of(a | b));
        BOOST_CHECK((a | b).intersects(b));
        BOOST_CHECK(not b.is_subset_of(a));
        BOOST_CHECK(not a.is_subset_of(b));
        BOOST_CHECK(not a.is_proper_subset_of(b));
        BOOST_CHECK(not b.is_proper_subset_of(a));
        BOOST_CHECK(not T({ 5 }).intersects(a));
        BOOST_CHECK(not a.intersects(T({ 5 })));
}

// The shifts translate: left grows the width to hold the result, right empties past it, and neither has the width as a precondition. [design.md#width-is-capacity]
BOOST_AUTO_TEST_CASE(TheShiftsTranslateWhateverTheWidth)
{
        auto const b = T({ 3, 5 });
        BOOST_CHECK((b << 300) == T({ 303, 305 }));
        BOOST_CHECK((b >> 4) == T({ 1 }));
        BOOST_CHECK((b >> 300).empty());
        BOOST_CHECK((T() << 3).empty());
}

BOOST_AUTO_TEST_SUITE_END()
