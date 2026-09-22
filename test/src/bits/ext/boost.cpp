//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/concepts.hpp>                                 // bit_sequence
#include <test/set/concepts.hpp>                                      // bit_set
#include <xstd/bits/detail/contiguous_bit_container.hpp>              // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/contiguous_block_range.hpp>                // contiguous_block_range
#include <xstd/bits/ext/boost.hpp>                                    // bit_small_set, bit_small_vector, small_bitset
#include <xstd/bits/ext/boost/detail/contiguous_bit_small_vector.hpp> // contiguous_bit_small_vector
#include <boost/container/new_allocator.hpp>                          // new_allocator
#include <boost/container/small_vector.hpp>                           // small_vector
#include <boost/test/unit_test.hpp>                                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                                   // regular, same_as, totally_ordered
#include <cstddef>                                                    // size_t
#include <cstdint>                                                    // uint8_t
#include <ranges>                                                     // bidirectional_range, random_access_range

// The one column whose storage comes from outside the standard library, kept off the umbrella so Boost stays opt-in.
BOOST_AUTO_TEST_SUITE(ExtBoost)

namespace {

inline constexpr auto N = 256UZ;

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

// What the umbrella is for: one include, and all three readings over the one column are reachable by name.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryReading)
{
        static_assert(test::set::bit_set<xstd::bit_small_set<N>>);
        static_assert(test::sequence::bit_sequence<xstd::bit_small_vector<N>>);
        static_assert(std::regular<xstd::small_bitset<N>> and std::totally_ordered<xstd::small_bitset<N>>);

        static_assert(std::ranges::bidirectional_range<xstd::bit_small_set<N>>);
        static_assert(std::ranges::random_access_range<xstd::bit_small_vector<N>>);
        static_assert(not std::ranges::range<xstd::small_bitset<N>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
