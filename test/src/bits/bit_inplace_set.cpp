//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR, has_inplace_vector
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#ifdef TEST_HAS_INPLACE_VECTOR
#include <test/set/concepts.hpp>                     // bit_set
#include <xstd/bits/set_adaptor.hpp>                 // set_adaptor
#include <xstd/bits/bit_inplace_set.hpp>             // basic_bit_inplace_set, bit_inplace_set
#include <xstd/bits/detail/block_inplace_vector.hpp> // block_inplace_vector
#include <xstd/bits/ownership.hpp>                   // ownership
#include <algorithm>                                 // equal
#include <concepts>                                  // same_as
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint8_t
#include <new>                                       // bad_alloc
#include <ranges>                                    // iota, to
#include <set>                                       // set
#endif

BOOST_AUTO_TEST_SUITE(BitInplaceSet)

#ifdef TEST_HAS_INPLACE_VECTOR

// A capacity of three whole blocks, so a key can sit past the width and still inside the capacity.
using T = xstd::basic_bit_inplace_set<std::uint8_t, 24>;

// Dependent, so an absent member is a false rather than a hard error.
template<class X>
constexpr bool has_capacity = requires (X const& x) { x.capacity(); };

// The set reading over a run-time width under a compile-time capacity, an alias and nothing more. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheInplaceSetIsTheSetAdaptorOverAnInplaceVectorOfBlocks)
{
        static_assert(std::same_as<T, xstd::set_adaptor<xstd::detail::bits::block_inplace_vector<std::uint8_t, 24>, xstd::ownership::owns>>);
        static_assert(std::same_as<xstd::bit_inplace_set<24>, xstd::basic_bit_inplace_set<std::size_t, 24>>);
        static_assert(test::set::bit_set<T>);
}

// Built from a range as std::set is, and ordered as std::set is.
BOOST_AUTO_TEST_CASE(ItIsBuiltAndOrderedLikeAStdSet)
{
        auto const s = std::views::iota(0UZ, 24UZ) | std::views::filter([](auto i) { return i % 5 == 0; }) | std::ranges::to<T>();
        auto const k = std::views::iota(0UZ, 24UZ) | std::views::filter([](auto i) { return i % 5 == 0; }) | std::ranges::to<std::set<std::size_t>>();
        BOOST_CHECK(std::ranges::equal(s, k));
        BOOST_CHECK_EQUAL(s.size(), k.size());
}

// A key past the width grows the width, exactly as the heap-backed set does, until the capacity stops it. [design.md#asking-is-total]
BOOST_AUTO_TEST_CASE(InsertingPastTheWidthGrowsItUpToTheCapacity)
{
        auto s = T();
        BOOST_CHECK(s.empty());

        auto const [ where, inserted ] = s.insert(20);
        BOOST_CHECK(inserted);
        BOOST_CHECK(*where == 20UZ);
        BOOST_CHECK(s.contains(20));
        BOOST_CHECK_EQUAL(s.size(), 1UZ);

        // Below the capacity but above the width, lookups stay total and answer no.
        BOOST_CHECK(not s.contains(23));
        BOOST_CHECK_EQUAL(s.erase(23), 0UZ);
}

// Past the capacity there is nowhere to grow, and the storage's bad_alloc reaches the caller. [design.md#growth]
BOOST_AUTO_TEST_CASE(InsertingPastTheCapacityThrowsBadAlloc)
{
        auto s = T();

        // max_size() is the positions there are to hold, which under a static capacity is that capacity, the same
        // answer the other two readings give over this storage. [design.md#max-size-is-the-bits]
        BOOST_CHECK_EQUAL(s.max_size(), 24UZ);
        static_assert(not has_capacity<T>);
        BOOST_CHECK_THROW(s.insert(24), std::bad_alloc);

        // The failed insert left the set empty, and a key past the capacity is still answerable.
        BOOST_CHECK(s.empty());
        BOOST_CHECK(not s.contains(24));
        BOOST_CHECK(s.find(24) == s.end());  // NOLINT(readability-container-contains)
}

// Width is capacity here as it is on the heap: two sets holding the same keys are equal whatever their widths. [design.md#width-is-capacity]
BOOST_AUTO_TEST_CASE(EqualSetsCompareEqualAtUnequalWidths)
{
        auto narrow = T();
        auto wide   = T();

        narrow.insert(3);
        wide.insert(20);
        wide.erase(20);
        wide.insert(3);

        BOOST_CHECK(narrow == wide);
        BOOST_CHECK(not (narrow < wide) and not (wide < narrow));
}

#else

// The column is its storage's: without std::inplace_vector there is no name to test, and saying so keeps the source from being empty. [design.md#the-inplace-column]
BOOST_AUTO_TEST_CASE(TheColumnIsAbsentWithItsStorage)
{
        static_assert(not test::has_inplace_vector);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
