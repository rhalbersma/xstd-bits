//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_concepts/bit_blocks.hpp>     // bit_blocks
#include <xstd/bits/detail/bit_block_container.hpp>  // bit_block_container, move_assigns_without_throwing_v, swaps_without_throwing_v
#include <xstd/bits/detail/bit_blocks_capacity.hpp>  // bit_blocks_capacity_v
#include <xstd/bits/detail/owned_bit_blocks.hpp>     // owned_bit_blocks
#include <xstd/bits/detail/resizable_bit_blocks.hpp> // resizable_bit_blocks
#include <xstd/bits/detail/small_blocks.hpp>         // small_allocator, small_blocks, small_blocks_allocator
#include <boost/test/unit_test.hpp>                  // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstdint>                                   // uint16_t, uint64_t
#include <memory>                                    // allocator
#include <memory_resource>                           // polymorphic_allocator
#include <span>                                      // dynamic_extent
#include <type_traits>                               // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v

#ifdef XSTD_BITS_SMALL_VECTOR_ABSL

#include <absl/container/inlined_vector.h> // InlinedVector

#else

#include <boost/container/new_allocator.hpp> // new_allocator
#include <boost/container/small_vector.hpp>  // small_vector

#endif

BOOST_AUTO_TEST_SUITE(Detail)
BOOST_AUTO_TEST_SUITE(SmallBlocks)

namespace {

using Blocks = xstd::bits::detail::small_blocks<std::uint16_t, 3, xstd::bits::detail::small_allocator<std::uint16_t>>;

} // namespace

// The storage the build chose, at the allocator that storage defaults to.
BOOST_AUTO_TEST_CASE(TheStorageIsTheOneTheBuildChose)
{
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(std::is_same_v<Blocks, absl::InlinedVector<std::uint16_t, 3, std::allocator<std::uint16_t>>>);
#else
        static_assert(std::is_same_v<Blocks, boost::container::small_vector<std::uint16_t, 3, boost::container::new_allocator<std::uint16_t>>>);
#endif
        BOOST_CHECK(true);
}

// Every small vector meets the storage contract as it comes, with no trait and no wrapper.
BOOST_AUTO_TEST_CASE(TheStorageMeetsTheStorageContract)
{
        static_assert(xstd::bit_blocks<Blocks> and xstd::bits::detail::owned_bit_blocks<Blocks> and xstd::bits::detail::resizable_bit_blocks<Blocks>);
        BOOST_CHECK(true);
}

// What it holds inline is only what it holds before it allocates, so it bounds nothing and the width runs free.
BOOST_AUTO_TEST_CASE(TheInlineCapacityBoundsNothing)
{
        static_assert(xstd::bits::detail::bit_blocks_capacity_v<Blocks> == std::dynamic_extent);
        static_assert(not xstd::bits::detail::bit_block_container<Blocks>::has_static_capacity);
        BOOST_CHECK(true);
}

// A std::pmr allocator differs between copies; Boost propagates it as its traits say, Abseil swaps and drops it.
BOOST_AUTO_TEST_CASE(AnAllocatorHoldsTheBlocksWhereTheStorageHonoursIt)
{
        static_assert(xstd::bits::detail::small_blocks_allocator<std::allocator<std::uint64_t>>);
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(not xstd::bits::detail::small_blocks_allocator<std::pmr::polymorphic_allocator<std::uint64_t>>);
#else
        static_assert(xstd::bits::detail::small_blocks_allocator<std::pmr::polymorphic_allocator<std::uint64_t>>);
#endif
        BOOST_CHECK(true);
}

// Abseil declares neither its move assignment nor its swap noexcept, though under std::allocator both only memcpy.
BOOST_AUTO_TEST_CASE(TheVehicleKnowsWhatTheStorageThrows)
{
        static_assert(std::is_nothrow_move_constructible_v<Blocks>);
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(not std::is_nothrow_move_assignable_v<Blocks> and not std::is_nothrow_swappable_v<Blocks>);
        static_assert(xstd::bits::detail::move_assigns_without_throwing_v<Blocks> and xstd::bits::detail::swaps_without_throwing_v<Blocks>);
#else
        static_assert(std::is_nothrow_move_assignable_v<Blocks> and std::is_nothrow_swappable_v<Blocks>);
        static_assert(xstd::bits::detail::move_assigns_without_throwing_v<Blocks> and not xstd::bits::detail::swaps_without_throwing_v<Blocks>);
#endif
        static_assert(std::is_nothrow_move_assignable_v<xstd::bits::detail::bit_block_container<Blocks>> and std::is_nothrow_swappable_v<xstd::bits::detail::bit_block_container<Blocks>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
