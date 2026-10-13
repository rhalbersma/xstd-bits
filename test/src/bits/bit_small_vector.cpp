//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/concepts.hpp>               // bit_sequence
#include <test/sequence/rotation.hpp>               // permutation_sweep
#include <xstd/bits/bit_small_vector.hpp>           // basic_bit_small_vector, bit_small_vector
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/num_blocks.hpp>          // num_blocks_v
#include <xstd/bits/detail/ownership.hpp>           // owned_bits_t
#include <xstd/bits/detail/small_blocks.hpp>        // small_allocator, small_blocks
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t
#include <memory>                                   // allocator
#include <memory_resource>                          // polymorphic_allocator
#include <ranges>                                   // iota, random_access_range
#include <type_traits>                              // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v

#ifndef XSTD_BITS_SMALL_VECTOR_ABSL

#include <boost/container/new_allocator.hpp> // new_allocator

#endif

BOOST_AUTO_TEST_SUITE(BitSmallVector)

namespace {

inline constexpr auto N = 256UZ;
using SmallVector       = xstd::bit_small_vector<N>;

template<class Alloc>
constexpr bool takes_allocator = requires { typename xstd::basic_bit_small_vector<std::size_t, N, Alloc>; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with it

} // namespace

// The short name is the general one at its defaults, whichever storage holds the blocks.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<SmallVector, xstd::basic_bit_small_vector<std::size_t, N>>);
        static_assert(test::sequence::bit_sequence<SmallVector>);
        static_assert(std::ranges::random_access_range<SmallVector>);
        BOOST_CHECK(true);
}

// The build's choice is in the type's own name, through the one inline namespace that choice declares.
BOOST_AUTO_TEST_CASE(TheStorageIsNamedByTheInlineNamespace)
{
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(std::same_as<SmallVector, xstd::with_absl::basic_bit_small_vector<std::size_t, N, std::allocator<std::size_t>>>);
#else
        static_assert(std::same_as<SmallVector, xstd::with_boost::basic_bit_small_vector<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
#endif
        BOOST_CHECK(true);
}

// N counts bits and the storage counts blocks, so the vehicle divides by the block width as the static column does.
BOOST_AUTO_TEST_CASE(TheCapacityIsBitsAndTheStorageIsBlocks)
{
        using Blocks = xstd::bits::detail::small_blocks<std::size_t, xstd::bits::detail::num_blocks_v<std::size_t, N>, xstd::bits::detail::small_allocator<std::size_t>>;
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<SmallVector>, xstd::bits::detail::bit_block_container<Blocks>>);
        BOOST_CHECK(true);
}

// Abseil ignores the propagation traits, so a std::pmr allocator, unequal between copies, is refused there.
BOOST_AUTO_TEST_CASE(AnAllocatorIsTakenWhereTheStorageHonoursIt)
{
        static_assert(takes_allocator<std::allocator<std::size_t>>);
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(not takes_allocator<std::pmr::polymorphic_allocator<std::size_t>>);
#else
        static_assert(takes_allocator<std::pmr::polymorphic_allocator<std::size_t>>);
#endif
        BOOST_CHECK(true);
}

// Deduced from the storage, pinned per storage: what each declares, or the vehicle states for it, the vector inherits.
BOOST_AUTO_TEST_CASE(TheMovesAndSwapAreAsNothrowAsTheStorage)
{
        static_assert(std::is_nothrow_move_constructible_v<SmallVector> and std::is_nothrow_move_assignable_v<SmallVector> and std::is_nothrow_swappable_v<SmallVector>);
#ifndef XSTD_BITS_SMALL_VECTOR_ABSL
        using PolymorphicVector = xstd::basic_bit_small_vector<std::size_t, N, std::pmr::polymorphic_allocator<std::size_t>>;
        static_assert(std::is_nothrow_move_constructible_v<PolymorphicVector> and not std::is_nothrow_move_assignable_v<PolymorphicVector>);
#endif
        BOOST_CHECK(true);
}

// rotate and reverse inline and spilled: every width through three blocks, whole and partial ones past the inline two.
BOOST_AUTO_TEST_CASE(ItRotatesAndReversesAsTheAlgorithmsDo)
{
        using V            = xstd::basic_bit_small_vector<std::uint8_t, 16>;
        auto disagreements = 0;
        for (auto const n : std::views::iota(0UZ, 18UZ)) {
                disagreements += test::sequence::permutation_sweep(V(n));
        }
        for (auto const n : {64UZ, 70UZ}) {
                disagreements += test::sequence::permutation_sweep(V(n));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

BOOST_AUTO_TEST_SUITE_END()
