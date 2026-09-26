//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/minimal_words.hpp>                        // minimal_words
#include <xstd/bits/bit_array.hpp>                       // bit_array
#include <xstd/bits/bit_set.hpp>                         // bit_set
#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bit_storage.hpp>                     // bit_storage, bit_storage_capacity_v, bit_storage_extent_v, owned_bit_storage, resizable_bit_storage
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t, uint16_t, uint32_t, uint64_t
#include <deque>                                         // deque
#include <list>                                          // list
#include <span>                                          // dynamic_extent, span
#include <type_traits>                                   // is_same_v
#include <vector>                                        // vector
#include <version>                                       // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitStorage)

namespace {

template<class W>
concept holds_words = requires { typename xstd::bits::detail::contiguous_bit_container<W>; };

template<class W>
concept names_a_view = requires { typename xstd::bit_set_view<W>; };

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::contiguous_bit_container<W, N>; };

} // namespace

// A word is bit storage, and so is a sized contiguous range of words: every storage the containers hold.
BOOST_AUTO_TEST_CASE(WordsAndContiguousRangesOfWordsAreBitStorage)
{
        static_assert(xstd::bit_storage<std::uint8_t> and xstd::bit_storage<std::uint64_t> and xstd::bit_storage<std::uint64_t const>);
        static_assert(xstd::bit_storage<std::array<std::uint16_t, 3>>);
        static_assert(xstd::bit_storage<std::vector<std::size_t>>);
        static_assert(xstd::bit_storage<std::span<std::uint32_t>> and xstd::bit_storage<std::span<std::uint32_t const, 2>>);
#ifdef __cpp_lib_inplace_vector
        static_assert(xstd::bit_storage<std::inplace_vector<std::uint16_t, 3>>);
#endif
        BOOST_CHECK(true);
}

// A packed container has bit storage and is not bit storage, and neither is anything not laid out as words.
BOOST_AUTO_TEST_CASE(EverythingElseIsNot)
{
        static_assert(not xstd::bit_storage<int> and not xstd::bit_storage<bool> and not xstd::bit_storage<double>);
        static_assert(not xstd::bit_storage<std::vector<int>> and not xstd::bit_storage<std::vector<bool>>);
        static_assert(not xstd::bit_storage<std::deque<std::uint32_t>> and not xstd::bit_storage<std::list<std::uint32_t>>);
        static_assert(not xstd::bit_storage<std::bitset<64>>);
        static_assert(not xstd::bit_storage<xstd::bit_set> and not xstd::bit_storage<xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

// The storage and the views are named by bit storage and nothing else.
BOOST_AUTO_TEST_CASE(TheStorageAndTheViewsAreNamedByBitStorage)
{
        static_assert(holds_words<std::vector<std::uint32_t>> and holds_words<std::array<std::uint64_t, 1>>);
        static_assert(names_a_view<std::uint64_t const> and names_a_view<std::span<std::uint32_t>>);
        static_assert(not holds_words<std::bitset<64>> and not holds_words<xstd::bit_set>);
        static_assert(not names_a_view<std::vector<bool>> and not names_a_view<int>);
        BOOST_CHECK(true);
}

// The width storage names by its type, which the views default to: fixed words have one, the rest do not.
BOOST_AUTO_TEST_CASE(TheExtentIsTheWidthTheTypeNames)
{
        static_assert(xstd::bit_storage_extent_v<std::uint8_t> == 8 and xstd::bit_storage_extent_v<std::uint64_t const> == 64);
        static_assert(xstd::bit_storage_extent_v<std::array<std::uint16_t, 3>> == 48 and xstd::bit_storage_extent_v<std::array<std::uint16_t, 3> const> == 48);
        static_assert(xstd::bit_storage_extent_v<std::span<std::uint32_t, 2>> == 64 and xstd::bit_storage_extent_v<std::span<std::uint32_t const, 2>> == 64);
        static_assert(xstd::bit_storage_extent_v<std::span<std::uint32_t>> == std::dynamic_extent);
        static_assert(xstd::bit_storage_extent_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bit_set_view<std::span<std::uint32_t>>, xstd::bit_set_view<std::span<std::uint32_t>, std::dynamic_extent>>);
        BOOST_CHECK(true);
}

// An owner takes what compares by its words and stays read-only through const; a span is neither, so views take it.
BOOST_AUTO_TEST_CASE(OwnedStorageIsAValueThatConstKeepsReadOnly)
{
        static_assert(xstd::owned_bit_storage<std::uint64_t> and xstd::owned_bit_storage<std::array<std::uint16_t, 3>>);
        static_assert(xstd::owned_bit_storage<std::vector<std::size_t>>);
        static_assert(not xstd::owned_bit_storage<std::span<std::uint32_t>> and not xstd::owned_bit_storage<std::span<std::uint32_t, 2>>);
        static_assert(not xstd::owned_bit_storage<std::uint64_t const> and not xstd::owned_bit_storage<std::array<std::uint16_t, 3> const>);
        static_assert(xstd::bit_storage<std::span<std::uint32_t>> and xstd::bit_storage<std::uint64_t const>);
        BOOST_CHECK(true);
}

// A run-time width grows its words, so an owner at one takes only storage that resizes; a fixed width takes any.
BOOST_AUTO_TEST_CASE(ARunTimeWidthOwnsOnlyStorageThatResizes)
{
        static_assert(xstd::resizable_bit_storage<std::vector<std::size_t>>);
        static_assert(not xstd::resizable_bit_storage<std::array<std::uint64_t, 2>> and not xstd::resizable_bit_storage<std::uint64_t>);
#ifdef __cpp_lib_inplace_vector

        static_assert(xstd::resizable_bit_storage<std::inplace_vector<std::uint16_t, 3>>);

#endif
        static_assert(xstd::resizable_bit_storage<test::minimal_words<std::uint32_t>>);
        static_assert(holds_extent<std::vector<std::size_t>, std::dynamic_extent>);
        static_assert(holds_extent<test::minimal_words<std::uint32_t>, std::dynamic_extent>);
        static_assert(holds_extent<std::array<std::uint64_t, 2>, 100> and not holds_extent<std::array<std::uint64_t, 2>, std::dynamic_extent>);
        static_assert(not holds_extent<std::uint64_t, std::dynamic_extent>);
        BOOST_CHECK(true);
}

// An owner's N is its bound: a fixed width, a constant capacity its blocks hold in whole, or none at all.
BOOST_AUTO_TEST_CASE(AnOwnersExtentIsItsWidthOrItsCapacity)
{
        static_assert(xstd::bit_storage_capacity_v<std::uint64_t> == 64 and xstd::bit_storage_capacity_v<std::array<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_storage_capacity_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(xstd::bit_storage_capacity_v<test::minimal_words<std::uint32_t>> == std::dynamic_extent);
#ifdef __cpp_lib_inplace_vector

        static_assert(xstd::bit_storage_capacity_v<std::inplace_vector<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_storage_extent_v<std::inplace_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bits::detail::contiguous_bit_container<std::inplace_vector<std::uint16_t, 3>>, xstd::bits::detail::contiguous_bit_container<std::inplace_vector<std::uint16_t, 3>, 48>>);

        // Any capacity the blocks hold in whole: stopping inside the last block, but never short of it.
        static_assert(holds_extent<std::inplace_vector<std::uint16_t, 3>, 33> and holds_extent<std::inplace_vector<std::uint16_t, 3>, 47>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, 32> and not holds_extent<std::inplace_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, std::dynamic_extent>);

#endif
        static_assert(not holds_extent<std::vector<std::size_t>, 64>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
