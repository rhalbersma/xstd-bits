//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                         // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_blocks.hpp>                         // minimal_blocks
#include <xstd/bits/bit_concepts/resizable_bit_blocks.hpp> // resizable_bit_blocks
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                           // array
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint16_t, uint32_t, uint64_t
#include <span>                                            // dynamic_extent
#include <vector>                                          // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(ResizableBitBlocks)

namespace {

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::bit_block_container<W, N>; };

} // namespace

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

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
