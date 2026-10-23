//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                          // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bit_type_traits/bit_storage_rebind.hpp> // bit_storage_rebind
#include <xstd/bits/ext/boost/bit_small_vector.hpp>         // IWYU pragma: keep; bit_storage_rebind for small_vector
#include <boost/container/new_allocator.hpp>                // new_allocator
#include <boost/container/small_vector.hpp>                 // small_vector
#include <boost/test/unit_test.hpp>                         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstdint>                                          // uint16_t, uint64_t, uint8_t
#include <memory>                                           // allocator
#include <type_traits>                                      // is_same_v
#include <vector>                                           // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitStorageRebind)

namespace {

template<class Blocks>
using with_byte_blocks = xstd::bit_storage_rebind<Blocks>::template with_block<std::uint8_t>;

template<class Blocks>
concept counts_blocks = requires { xstd::bit_storage_rebind<Blocks>::block_count; };

} // namespace

// A sequence over an allocator takes the allocator along, rebound as std::allocator_traits rebinds it.
BOOST_AUTO_TEST_CASE(AnAllocatorIsReboundWithTheBlock)
{
        static_assert(std::is_same_v<with_byte_blocks<std::vector<std::uint64_t>>, std::vector<std::uint8_t, std::allocator<std::uint8_t>>>);
        static_assert(std::is_same_v<with_byte_blocks<std::pmr::vector<std::uint64_t>>, std::pmr::vector<std::uint8_t>>);
        static_assert(std::is_same_v<with_byte_blocks<boost::container::small_vector<std::uint64_t, 2, boost::container::new_allocator<std::uint64_t>>>, boost::container::small_vector<std::uint8_t, 2, boost::container::new_allocator<std::uint8_t>>>);
        BOOST_CHECK(true);
}

// A count of blocks in the type is the storage's own, and another count names the same storage resized.
BOOST_AUTO_TEST_CASE(ACountOfBlocksIsKeptAndChanged)
{
        using small = boost::container::small_vector<std::uint16_t, 3>;
        static_assert(xstd::bit_storage_rebind<small>::block_count == 3UZ);
        static_assert(std::is_same_v<xstd::bit_storage_rebind<small>::with_block_count<5>, boost::container::small_vector<std::uint16_t, 5>>);
        static_assert(std::is_same_v<with_byte_blocks<small>, boost::container::small_vector<std::uint8_t, 3>>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::bit_storage_rebind<std::inplace_vector<std::uint64_t, 2>>::block_count == 2UZ);
        static_assert(std::is_same_v<xstd::bit_storage_rebind<std::inplace_vector<std::uint64_t, 2>>::with_block_count<4>, std::inplace_vector<std::uint64_t, 4>>);
#endif
        BOOST_CHECK(true);
}

// A heap of blocks counts none in its type, so it has no count to change.
BOOST_AUTO_TEST_CASE(AHeapOfBlocksHasNoCount)
{
        static_assert(not counts_blocks<std::vector<std::uint64_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
