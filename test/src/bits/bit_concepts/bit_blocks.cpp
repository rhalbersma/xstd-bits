//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_concepts/bit_blocks.hpp>    // bit_blocks
#include <xstd/bits/bit_set.hpp>                    // bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                    // array
#include <bitset>                                   // bitset
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint16_t, uint32_t, uint64_t, uint8_t
#include <deque>                                    // deque
#include <list>                                     // list
#include <span>                                     // span
#include <vector>                                   // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitBlocks)

namespace {

template<class W>
concept holds_blocks = requires { typename xstd::bits::detail::bit_block_container<W>; };

template<class W>
concept names_a_view = requires { typename xstd::bit_set_view<W>; };

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

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
