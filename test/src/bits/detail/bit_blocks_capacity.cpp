//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                         // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_blocks.hpp>                         // minimal_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // IWYU pragma: keep; bit_blocks_extent_v, read where TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <xstd/bits/detail/bit_blocks_capacity.hpp>        // bit_blocks_capacity_v
#include <xstd/bits/detail/resizable_bit_blocks.hpp>       // resizable_bit_blocks
#include <boost/container/small_vector.hpp>                // small_vector
#include <boost/container/static_vector.hpp>               // static_vector
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                           // array
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint16_t, uint32_t, uint64_t
#include <span>                                            // dynamic_extent
#include <type_traits>                                     // is_same_v
#include <vector>                                          // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(Detail)
BOOST_AUTO_TEST_SUITE(BitBlocksCapacity)

namespace {

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::bit_block_container<W, N>; };

} // namespace

// An owner's N is its bound: a fixed width, a constant capacity its blocks hold in whole, or none at all.
BOOST_AUTO_TEST_CASE(AnOwnersExtentIsItsWidthOrItsCapacity)
{
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<std::uint64_t> == 64 and xstd::bits::detail::bit_blocks_capacity_v<std::array<std::uint16_t, 3>> == 48);
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<test::minimal_blocks<std::uint32_t>> == std::dynamic_extent);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::bits::detail::bit_blocks_capacity_v<std::inplace_vector<std::uint16_t, 3>> == 48);
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
        static_assert(xstd::bits::detail::resizable_bit_blocks<boost::container::static_vector<std::uint16_t, 3>>);
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<boost::container::static_vector<std::uint16_t, 3>> == 48);
        static_assert(std::is_same_v<xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>>, xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>, 48>>);
        static_assert(holds_extent<boost::container::static_vector<std::uint16_t, 3>, 33> and not holds_extent<boost::container::static_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<boost::container::static_vector<std::uint16_t, 3>, std::dynamic_extent>);

        // A small_vector's static_capacity is only what it holds before it allocates, so it bounds nothing.
        static_assert(xstd::bits::detail::resizable_bit_blocks<boost::container::small_vector<std::uint16_t, 3>>);
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<boost::container::small_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(holds_extent<boost::container::small_vector<std::uint16_t, 3>, std::dynamic_extent>);
        static_assert(not holds_extent<boost::container::small_vector<std::uint16_t, 3>, 48>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
