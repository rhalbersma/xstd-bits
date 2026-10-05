//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_blocks.hpp>                  // minimal_blocks
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_blocks.hpp>                 // bit_block, bit_block_range, bit_blocks, bit_blocks_capacity_v, bit_blocks_extent_v, owned_bit_blocks, resizable_bit_blocks, smallest_block_t
#include <xstd/bits/bit_set.hpp>                    // bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <boost/container/small_vector.hpp>         // small_vector
#include <boost/container/static_vector.hpp>        // static_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                    // array
#include <bitset>                                   // bitset
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <deque>                                    // deque
#include <list>                                     // list
#include <span>                                     // dynamic_extent, span
#include <type_traits>                              // is_same_v
#include <vector>                                   // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitBlocks)

namespace {

template<class W>
concept holds_blocks = requires { typename xstd::bits::detail::bit_block_container<W>; };

template<class W>
concept names_a_view = requires { typename xstd::bit_set_view<W>; };

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::bit_block_container<W, N>; };

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_words        = std::uint64_t[4];       // NOLINT(modernize-avoid-c-arrays): the storage under test
using three_const_words = std::uint16_t const[3]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// A block is bit storage, and so is a sized contiguous range of blocks: every storage the containers hold.
BOOST_AUTO_TEST_CASE(BlocksAndContiguousRangesOfBlocksAreBitStorage)
{
        static_assert(xstd::bit_blocks<std::uint8_t> and xstd::bit_blocks<std::uint64_t> and xstd::bit_blocks<std::uint64_t const>);
        static_assert(xstd::bit_blocks<std::array<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks<std::vector<std::size_t>>);
        static_assert(xstd::bit_blocks<std::span<std::uint32_t>> and xstd::bit_blocks<std::span<std::uint32_t const, 2>>);
        static_assert(xstd::bit_blocks<four_words> and xstd::bit_blocks<three_const_words>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::bit_blocks<std::inplace_vector<std::uint16_t, 3>>);
#endif
        BOOST_CHECK(true);
}

// Bit blocks are one block or a range of them, and the two halves are named apart.
BOOST_AUTO_TEST_CASE(BitBlocksAreABlockOrARangeOfThem)
{
        static_assert(xstd::bit_block<std::uint64_t> and xstd::bit_block<std::uint64_t const> and xstd::bit_block<std::uint8_t volatile>);
        static_assert(not xstd::bit_block<int> and not xstd::bit_block<bool> and not xstd::bit_block<std::array<std::uint64_t, 1>>);
        static_assert(xstd::bit_block_range<std::array<std::uint16_t, 3>> and xstd::bit_block_range<std::span<std::uint32_t const>>);
        static_assert(xstd::bit_block_range<std::vector<std::size_t>> and xstd::bit_block_range<four_words>);
        static_assert(not xstd::bit_block_range<std::uint64_t> and not xstd::bit_block_range<std::deque<std::uint32_t>>);
        BOOST_CHECK(true);
}

// A packed container has bit storage and is not bit storage, and neither is anything not laid out as blocks.
BOOST_AUTO_TEST_CASE(EverythingElseIsNot)
{
        static_assert(not xstd::bit_blocks<int> and not xstd::bit_blocks<bool> and not xstd::bit_blocks<double>);
        static_assert(not xstd::bit_blocks<std::vector<int>> and not xstd::bit_blocks<std::vector<bool>>);
        static_assert(not xstd::bit_blocks<std::deque<std::uint32_t>> and not xstd::bit_blocks<std::list<std::uint32_t>>);
        static_assert(not xstd::bit_blocks<std::bitset<64>>);
        static_assert(not xstd::bit_blocks<xstd::bit_set> and not xstd::bit_blocks<xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

// The storage and the views are named by bit storage and nothing else.
BOOST_AUTO_TEST_CASE(TheStorageAndTheViewsAreNamedByBitStorage)
{
        static_assert(holds_blocks<std::vector<std::uint32_t>> and holds_blocks<std::array<std::uint64_t, 1>>);
        static_assert(names_a_view<std::uint64_t const> and names_a_view<std::span<std::uint32_t>>);
        static_assert(not holds_blocks<std::bitset<64>> and not holds_blocks<xstd::bit_set>);
        static_assert(not names_a_view<std::vector<bool>> and not names_a_view<int>);
        BOOST_CHECK(true);
}

// The width storage names by its type, which the views default to: fixed blocks have one, the rest do not.
BOOST_AUTO_TEST_CASE(TheExtentIsTheWidthTheTypeNames)
{
        static_assert(xstd::bit_blocks_extent_v<std::uint8_t> == 8 and xstd::bit_blocks_extent_v<std::uint64_t const> == 64);
        static_assert(xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3>> == 48 and xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3> const> == 48);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t, 2>> == 64 and xstd::bit_blocks_extent_v<std::span<std::uint32_t const, 2>> == 64);
        static_assert(xstd::bit_blocks_extent_v<four_words> == 256 and xstd::bit_blocks_extent_v<three_const_words> == 48);
        static_assert(xstd::bit_blocks_extent_v<four_words> == xstd::bit_blocks_extent_v<std::array<std::uint64_t, 4>>);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t>> == std::dynamic_extent);
        static_assert(xstd::bit_blocks_extent_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bit_set_view<std::span<std::uint32_t>>, xstd::bit_set_view<std::span<std::uint32_t>, std::dynamic_extent>>);
        BOOST_CHECK(true);
}

// The narrowest fixed-width block holding N bits, the widest taking every N above it in several blocks.
BOOST_AUTO_TEST_CASE(TheSmallestBlockIsTheNarrowestHoldingTheWidth)
{
        static_assert(std::is_same_v<xstd::smallest_block_t<0>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<8>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<9>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<16>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<17>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<32>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<33>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<64>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<65>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::smallest_block_t<1000>, std::uint64_t>);
        BOOST_CHECK(true);
}

// An owner takes what compares by its blocks and stays read-only through const; a span is neither, so views take it.
BOOST_AUTO_TEST_CASE(OwnedStorageIsAValueThatConstKeepsReadOnly)
{
        static_assert(xstd::owned_bit_blocks<std::uint64_t> and xstd::owned_bit_blocks<std::array<std::uint16_t, 3>>);
        static_assert(xstd::owned_bit_blocks<std::vector<std::size_t>>);
        static_assert(not xstd::owned_bit_blocks<std::span<std::uint32_t>> and not xstd::owned_bit_blocks<std::span<std::uint32_t, 2>>);
        static_assert(not xstd::owned_bit_blocks<std::uint64_t const> and not xstd::owned_bit_blocks<std::array<std::uint16_t, 3> const>);
        static_assert(xstd::bit_blocks<std::span<std::uint32_t>> and xstd::bit_blocks<std::uint64_t const>);

        // A built-in array is bit storage and no value: it neither assigns nor compares, so no owner holds one.
        static_assert(not xstd::owned_bit_blocks<four_words> and not xstd::owned_bit_blocks<three_const_words>);
        static_assert(not holds_blocks<four_words> and not holds_extent<four_words, 256>);
        BOOST_CHECK(true);
}

// A run-time width grows its blocks, so an owner at one takes only storage that resizes; a fixed width takes any.
BOOST_AUTO_TEST_CASE(ARunTimeWidthOwnsOnlyStorageThatResizes)
{
        static_assert(xstd::resizable_bit_blocks<std::vector<std::size_t>>);
        static_assert(not xstd::resizable_bit_blocks<std::array<std::uint64_t, 2>> and not xstd::resizable_bit_blocks<std::uint64_t>);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::resizable_bit_blocks<std::inplace_vector<std::uint16_t, 3>>);

#endif
        static_assert(xstd::resizable_bit_blocks<test::minimal_blocks<std::uint32_t>>);
        static_assert(holds_extent<std::vector<std::size_t>, std::dynamic_extent>);
        static_assert(holds_extent<test::minimal_blocks<std::uint32_t>, std::dynamic_extent>);
        static_assert(holds_extent<std::array<std::uint64_t, 2>, 100> and not holds_extent<std::array<std::uint64_t, 2>, std::dynamic_extent>);
        static_assert(not holds_extent<std::uint64_t, std::dynamic_extent>);
        BOOST_CHECK(true);
}

// An owner's N is its bound: a fixed width, a constant capacity its blocks hold in whole, or none at all.
BOOST_AUTO_TEST_CASE(AnOwnersExtentIsItsWidthOrItsCapacity)
{
        static_assert(xstd::bit_blocks_capacity_v<std::uint64_t> == 64 and xstd::bit_blocks_capacity_v<std::array<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_blocks_capacity_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(xstd::bit_blocks_capacity_v<test::minimal_blocks<std::uint32_t>> == std::dynamic_extent);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::bit_blocks_capacity_v<std::inplace_vector<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_blocks_extent_v<std::inplace_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bits::detail::bit_block_container<std::inplace_vector<std::uint16_t, 3>>, xstd::bits::detail::bit_block_container<std::inplace_vector<std::uint16_t, 3>, 48>>);

        // Any capacity the blocks hold in whole: stopping inside the last block, but never short of it.
        static_assert(holds_extent<std::inplace_vector<std::uint16_t, 3>, 33> and holds_extent<std::inplace_vector<std::uint16_t, 3>, 47>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, 32> and not holds_extent<std::inplace_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, std::dynamic_extent>);

#endif
        static_assert(not holds_extent<std::vector<std::size_t>, 64>);
        BOOST_CHECK(true);
}

// A static capacity() callable only at run time still bounds the type, through the static_capacity beside it.
BOOST_AUTO_TEST_CASE(AStaticVectorsCapacityIsItsStaticCapacity)
{
        static_assert(xstd::resizable_bit_blocks<boost::container::static_vector<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks_capacity_v<boost::container::static_vector<std::uint16_t, 3>> == 48);
        static_assert(std::is_same_v<xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>>, xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>, 48>>);
        static_assert(holds_extent<boost::container::static_vector<std::uint16_t, 3>, 33> and not holds_extent<boost::container::static_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<boost::container::static_vector<std::uint16_t, 3>, std::dynamic_extent>);

        // A small_vector's static_capacity is only what it holds before it allocates, so it bounds nothing.
        static_assert(xstd::resizable_bit_blocks<boost::container::small_vector<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks_capacity_v<boost::container::small_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(holds_extent<boost::container::small_vector<std::uint16_t, 3>, std::dynamic_extent>);
        static_assert(not holds_extent<boost::container::small_vector<std::uint16_t, 3>, 48>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
