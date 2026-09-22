//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/concepts.hpp>                    // bit_sequence
#include <test/set/concepts.hpp>                         // bit_set
#include <xstd/bits/detail/contiguous_block_range.hpp>   // contiguous_block_range
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/bits/ext/boost.hpp>                       // basic_bit_small_set, basic_bit_small_vector, basic_small_bitset,
                                                         // bit_small_set, bit_small_vector, contiguous_bit_small_vector, small_bitset
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                     // ranges::equal
#include <bitset>                                        // bitset
#include <concepts>                                      // regular, same_as, totally_ordered
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <memory>                                        // allocator
#include <ranges>                                        // bidirectional_range, random_access_range
#include <set>                                           // set
#include <vector>                                        // vector

// The one column whose storage comes from outside the standard library, kept off the umbrella so Boost stays opt-in.
BOOST_AUTO_TEST_SUITE(ExtBoost)

namespace {

inline constexpr auto N = 256UZ;

using SmallSet = xstd::bit_small_set<N>;
using SmallVector = xstd::bit_small_vector<N>;
using SmallBitset = xstd::small_bitset<N>;

// An allocation count the container cannot reach around, which is what tells an inline width from a heap one.
inline auto allocations = 0;

template<class T>
struct counting_allocator
{
        using value_type = T;

        counting_allocator() noexcept = default;

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

        [[nodiscard]] friend auto operator==(counting_allocator const&, counting_allocator const&) noexcept -> bool = default;
};

using CountedSet = xstd::basic_bit_small_set<std::size_t, N, counting_allocator<std::size_t>>;

} // namespace

// A back end joins on the block concept alone, which is the whole of what a storage has to satisfy.
BOOST_AUTO_TEST_CASE(TheSmallVectorIsBlocksAStorageCanHold)
{
        static_assert(xstd::detail::bits::contiguous_block_range<boost::container::small_vector<std::size_t, 4>>);
        BOOST_CHECK(true);
}

// N counts bits and the storage counts blocks, so the vehicle divides by the block width the way the static column does.
BOOST_AUTO_TEST_CASE(TheCapacityIsBitsAndTheStorageIsBlocks)
{
        using Blocks = boost::container::small_vector<std::size_t, xstd::detail::bits::num_blocks_v<std::size_t, N>, boost::container::new_allocator<std::size_t>>;
        static_assert(std::same_as<xstd::detail::bits::contiguous_bit_small_vector<std::size_t, N, boost::container::new_allocator<std::size_t>>, xstd::detail::bits::contiguous_bit_container<Blocks>>);
        static_assert(xstd::detail::bits::num_blocks_v<std::uint8_t, 24> == 3);
        BOOST_CHECK(true);
}

// The short name is the general one at its defaults, and the allocator defaulted to is the storage's own.
BOOST_AUTO_TEST_CASE(TheShortNamesAreTheGeneralOnesAtTheirDefaults)
{
        static_assert(std::same_as<SmallSet, xstd::basic_bit_small_set<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        static_assert(std::same_as<SmallVector, xstd::basic_bit_small_vector<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        static_assert(std::same_as<SmallBitset, xstd::basic_small_bitset<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        BOOST_CHECK(true);
}

// One column, three readings: the storage says where the bits live and says nothing about how they are read.
BOOST_AUTO_TEST_CASE(EveryReadingInstantiatesOverIt)
{
        static_assert(test::set::bit_set<SmallSet>);
        static_assert(test::sequence::bit_sequence<SmallVector>);
        static_assert(std::regular<SmallBitset> and std::totally_ordered<SmallBitset>);
        static_assert(std::ranges::bidirectional_range<SmallSet>);
        static_assert(std::ranges::random_access_range<SmallVector>);
        static_assert(std::regular<xstd::basic_bit_small_set<std::uint8_t, 24>>);
        BOOST_CHECK(true);
}

// Keys on both sides of the inline capacity, because that boundary is what this storage has and the others do not.
BOOST_AUTO_TEST_CASE(TheSetReadingAnswersStdSetAcrossTheBoundary)
{
        auto oracle = std::set<std::size_t>();
        auto ours = SmallSet();
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

BOOST_AUTO_TEST_CASE(TheSequenceReadingAnswersVectorBool)
{
        auto oracle = std::vector<bool>(300, false);
        auto ours = SmallVector(300);
        for (auto const i : {5UZ, 100UZ, 299UZ}) {
                oracle[i] = true;
                ours[i] = true;
        }
        BOOST_CHECK_EQUAL(ours.size(), oracle.size());
        BOOST_CHECK(std::ranges::equal(ours, oracle));
}

BOOST_AUTO_TEST_CASE(TheBitsetReadingAnswersStdBitset)
{
        auto oracle = std::bitset<300>();
        auto ours = SmallBitset(300);
        for (auto const i : {5UZ, 100UZ, 299UZ}) {
                oracle.set(i);
                ours.set(i);
        }
        BOOST_CHECK_EQUAL(ours.count(), oracle.count());
        BOOST_CHECK_EQUAL(ours.to_string(), oracle.to_string());

        ours <<= 7;
        oracle <<= 7;
        BOOST_CHECK_EQUAL(ours.to_string(), oracle.to_string());
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
