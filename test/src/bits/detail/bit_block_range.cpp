//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/bit_block_range.hpp> // bit_block_range
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                // array
#include <cstddef>                              // size_t
#include <cstdint>                              // uint16_t, uint32_t, uint64_t
#include <deque>                                // deque
#include <span>                                 // span
#include <vector>                               // vector

BOOST_AUTO_TEST_SUITE(Detail)
BOOST_AUTO_TEST_SUITE(BitBlockRange)

namespace {

// A built-in array of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks = std::uint64_t[4]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// A sized contiguous range of blocks, a built-in array among them; a block alone and a deque of blocks are none.
BOOST_AUTO_TEST_CASE(ARangeOfBlocksIsSizedAndContiguous)
{
        static_assert(xstd::bits::detail::bit_block_range<std::array<std::uint16_t, 3>> and xstd::bits::detail::bit_block_range<std::span<std::uint32_t const>>);
        static_assert(xstd::bits::detail::bit_block_range<std::vector<std::size_t>> and xstd::bits::detail::bit_block_range<four_blocks>);
        static_assert(not xstd::bits::detail::bit_block_range<std::uint64_t> and not xstd::bits::detail::bit_block_range<std::deque<std::uint32_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
