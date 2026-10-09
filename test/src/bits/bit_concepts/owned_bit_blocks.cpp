//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_concepts/bit_blocks.hpp>       // bit_blocks
#include <xstd/bits/bit_concepts/owned_bit_blocks.hpp> // owned_bit_blocks
#include <xstd/bits/detail/bit_block_container.hpp>    // bit_block_container
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                       // array
#include <cstddef>                                     // size_t
#include <cstdint>                                     // uint16_t, uint32_t, uint64_t
#include <span>                                        // span
#include <vector>                                      // vector

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(OwnedBitBlocks)

namespace {

template<class W>
concept holds_blocks = requires { typename xstd::bits::detail::bit_block_container<W>; };

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::bit_block_container<W, N>; };

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks        = std::uint64_t[4];       // NOLINT(modernize-avoid-c-arrays): the storage under test
using three_const_blocks = std::uint16_t const[3]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// An owner takes what compares by its blocks and stays read-only through const; a span is neither, so views take it.
BOOST_AUTO_TEST_CASE(OwnedStorageIsAValueThatConstKeepsReadOnly)
{
        static_assert(xstd::owned_bit_blocks<std::uint64_t> and xstd::owned_bit_blocks<std::array<std::uint16_t, 3>>);
        static_assert(xstd::owned_bit_blocks<std::vector<std::size_t>>);
        static_assert(not xstd::owned_bit_blocks<std::span<std::uint32_t>> and not xstd::owned_bit_blocks<std::span<std::uint32_t, 2>>);
        static_assert(not xstd::owned_bit_blocks<std::uint64_t const> and not xstd::owned_bit_blocks<std::array<std::uint16_t, 3> const>);
        static_assert(xstd::bit_blocks<std::span<std::uint32_t>> and xstd::bit_blocks<std::uint64_t const>);

        // A built-in array is bit storage and no value: it neither assigns nor compares, so no owner holds one.
        static_assert(not xstd::owned_bit_blocks<four_blocks> and not xstd::owned_bit_blocks<three_const_blocks>);
        static_assert(not holds_blocks<four_blocks> and not holds_extent<four_blocks, 256>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
