//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/concepts.hpp>                    // bit_set
#include <xstd/bits/bit_key_mapping.hpp>            // bit_key_mapping
#include <xstd/bits/bit_small_set.hpp>              // basic_bit_small_set, bit_small_set
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/num_blocks.hpp>          // num_blocks_v
#include <xstd/bits/detail/ownership.hpp>           // owned_bits_t
#include <xstd/bits/detail/small_blocks.hpp>        // small_allocator, small_blocks
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                // ranges::equal
#include <concepts>                                 // regular, same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t
#include <functional>                               // less
#include <memory>                                   // allocator
#include <memory_resource>                          // polymorphic_allocator
#include <ranges>                                   // bidirectional_range
#include <set>                                      // set
#include <type_traits>                              // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v

#ifndef XSTD_BITS_SMALL_VECTOR_ABSL

#include <boost/container/new_allocator.hpp> // new_allocator

#endif

BOOST_AUTO_TEST_SUITE(BitSmallSet)

namespace {

inline constexpr auto N = 256UZ;
using SmallSet          = xstd::bit_small_set<N>;

// An allocation count the container cannot reach around, which is what tells an inline width from a heap one.
inline auto allocations = 0;

template<class T>
struct counting_allocator
{
        using value_type = T;

        counting_allocator() = default;

        template<class U>
        explicit constexpr counting_allocator(counting_allocator<U> const&) noexcept
        {}

        [[nodiscard]] auto allocate(std::size_t n)
                -> T*
        {
                ++allocations;
                return std::allocator<T>().allocate(n);
        }

        auto deallocate(T* p, std::size_t n) noexcept
                -> void
        {
                std::allocator<T>().deallocate(p, n);
        }

        [[nodiscard]] friend auto operator==(counting_allocator const&, counting_allocator const&) -> bool = default;
};

using CountedSet = xstd::basic_bit_small_set<std::size_t, std::size_t, N, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, counting_allocator<std::size_t>>; // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator

template<class Alloc>
constexpr bool takes_allocator = requires { typename xstd::basic_bit_small_set<std::size_t, std::size_t, N, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, Alloc>; }; // NOLINT(modernize-use-transparent-functors, readability-redundant-typename): the default comparator, spelled to reach the allocator; a type-requirement is spelled with it

} // namespace

// The short name is the general one at its defaults, whichever storage holds the blocks.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<SmallSet, xstd::basic_bit_small_set<std::size_t, std::size_t, N>>);
        static_assert(test::set::bit_set<SmallSet>);
        static_assert(std::ranges::bidirectional_range<SmallSet>);
        static_assert(std::regular<xstd::basic_bit_small_set<std::size_t, std::uint8_t, 24>>);
        BOOST_CHECK(true);
}

// The build's choice is in the type's own name, through the one inline namespace that choice declares.
BOOST_AUTO_TEST_CASE(TheStorageIsNamedByTheInlineNamespace)
{
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(std::same_as<SmallSet, xstd::with_absl::basic_bit_small_set<std::size_t, std::size_t, N, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, std::allocator<std::size_t>>>); // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator
#else
        static_assert(std::same_as<SmallSet, xstd::with_boost::basic_bit_small_set<std::size_t, std::size_t, N, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, boost::container::new_allocator<std::size_t>>>); // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator
#endif
        BOOST_CHECK(true);
}

// N counts bits and the storage counts blocks, so the vehicle divides by the block width as the static column does.
BOOST_AUTO_TEST_CASE(TheCapacityIsBitsAndTheStorageIsBlocks)
{
        using Blocks = xstd::bits::detail::small_blocks<std::size_t, xstd::bits::detail::num_blocks_v<std::size_t, N>, xstd::bits::detail::small_allocator<std::size_t>>;
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<SmallSet>, xstd::bits::detail::bit_block_container<Blocks>>);
        BOOST_CHECK(true);
}

// Abseil ignores the propagation traits, so a std::pmr allocator, unequal between copies, is refused there.
BOOST_AUTO_TEST_CASE(AnAllocatorIsTakenWhereTheStorageHonoursIt)
{
        static_assert(takes_allocator<std::allocator<std::size_t>> and takes_allocator<counting_allocator<std::size_t>>);
#ifdef XSTD_BITS_SMALL_VECTOR_ABSL
        static_assert(not takes_allocator<std::pmr::polymorphic_allocator<std::size_t>>);
#else
        static_assert(takes_allocator<std::pmr::polymorphic_allocator<std::size_t>>);
#endif
        BOOST_CHECK(true);
}

// Deduced from the storage, pinned per storage: what each declares, or the vehicle states for it, the set inherits.
BOOST_AUTO_TEST_CASE(TheMovesAndSwapAreAsNothrowAsTheStorage)
{
        static_assert(std::is_nothrow_move_constructible_v<SmallSet> and std::is_nothrow_move_assignable_v<SmallSet> and std::is_nothrow_swappable_v<SmallSet>);
#ifndef XSTD_BITS_SMALL_VECTOR_ABSL
        using PolymorphicSet = xstd::basic_bit_small_set<std::size_t, std::size_t, N, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, std::pmr::polymorphic_allocator<std::size_t>>; // NOLINT(modernize-use-transparent-functors): the default comparator, spelled to reach the allocator
        static_assert(std::is_nothrow_move_constructible_v<PolymorphicSet> and not std::is_nothrow_move_assignable_v<PolymorphicSet>);
#endif
        BOOST_CHECK(true);
}

// Keys on both sides of the inline capacity, because that boundary is what this storage has and the others do not.
BOOST_AUTO_TEST_CASE(TheSetReadingAnswersStdSetAcrossTheBoundary)
{
        auto oracle = std::set<std::size_t>();
        auto ours   = SmallSet();
        for (auto const i : {0UZ, 1UZ, 63UZ, 64UZ, 255UZ, 256UZ, 1000UZ}) {
                oracle.insert(i);
                ours.insert(i);
        }
        BOOST_CHECK_EQUAL(ours.size(), oracle.size());
        BOOST_CHECK(std::ranges::equal(ours, oracle));

        ours.erase(64);
        oracle.erase(64);
        BOOST_CHECK(std::ranges::equal(ours, oracle));
}

// The inline buffer is the only reason to reach for this storage, so the test is that a narrow width never allocates.
BOOST_AUTO_TEST_CASE(AWidthInsideTheCapacityStaysOffTheHeap)
{
        allocations = 0;
        {
                auto inside = CountedSet();
                inside.insert(3);
                inside.insert(N - 1);
                BOOST_CHECK(inside.contains(3) and inside.contains(N - 1));
        }
        BOOST_CHECK_EQUAL(allocations, 0);

        allocations = 0;
        {
                auto beyond = CountedSet();
                beyond.insert(5 * N);
                BOOST_CHECK(beyond.contains(5 * N));
        }
        BOOST_CHECK(allocations > 0);
}

BOOST_AUTO_TEST_SUITE_END()
