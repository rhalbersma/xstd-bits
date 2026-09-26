//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/concepts.hpp>                    // bit_sequence
#include <test/set/concepts.hpp>                         // bit_set
#include <xstd/bits/bit_storage.hpp>                     // bit_storage, owned_bit_storage, resizable_bit_storage
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/bits/ext/boost.hpp>                       // bit_small_set, bit_small_vector, small_bitset
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <boost/container/static_vector.hpp>             // static_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                      // regular, same_as, totally_ordered
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <memory_resource>                               // polymorphic_allocator
#include <ranges>                                        // bidirectional_range, random_access_range
#include <type_traits>                                   // is_nothrow_move_assignable_v, is_nothrow_move_constructible_v

// The one column whose storage comes from outside the standard library, kept off the umbrella so Boost stays opt-in.
BOOST_AUTO_TEST_SUITE(ExtBoost)

namespace {

inline constexpr auto N = 256UZ;

// Three bytes held inline: a storage from outside the standard library, with a capacity of 24 bits.
using small_words = boost::container::static_vector<std::uint8_t, 3>;

} // namespace

// A back end joins on the block concept alone, which is the whole of what a storage has to satisfy.
BOOST_AUTO_TEST_CASE(TheSmallVectorIsBlocksAStorageCanHold)
{
        static_assert(xstd::owned_bit_storage<boost::container::small_vector<std::size_t, 4>>);
        static_assert(xstd::bit_storage<boost::container::small_vector<std::size_t, 4>>);
        static_assert(xstd::bit_storage<boost::container::small_vector<std::uint8_t, 4, boost::container::new_allocator<std::uint8_t>>>);
        BOOST_CHECK(true);
}

// N counts bits and the storage counts blocks, so the vehicle divides by the block width as the static column does.
BOOST_AUTO_TEST_CASE(TheCapacityIsBitsAndTheStorageIsBlocks)
{
        using Blocks = boost::container::small_vector<std::size_t, xstd::bits::detail::num_blocks_v<std::size_t, N>, boost::container::new_allocator<std::size_t>>;
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::bit_small_vector<N>>, xstd::bits::detail::contiguous_bit_container<Blocks>>);
        static_assert(xstd::bits::detail::num_blocks_v<std::uint8_t, 24> == 3);
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

// The implicit moves ask the small vector, whose move assignment may throw under a polymorphic allocator.
BOOST_AUTO_TEST_CASE(TheMovesAreAsNothrowAsTheSmallVectors)
{
        static_assert(std::is_nothrow_move_constructible_v<xstd::bit_small_set<N>> and std::is_nothrow_move_assignable_v<xstd::bit_small_set<N>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::bit_small_vector<N>> and std::is_nothrow_move_assignable_v<xstd::bit_small_vector<N>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::small_bitset<N>> and std::is_nothrow_move_assignable_v<xstd::small_bitset<N>>);

        using allocator_type = std::pmr::polymorphic_allocator<std::size_t>;
        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_bit_small_set<std::size_t, N, allocator_type>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_bit_small_vector<std::size_t, N, allocator_type>>);
        static_assert(std::is_nothrow_move_constructible_v<xstd::basic_small_bitset<std::size_t, N, allocator_type>>);
        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_bit_small_set<std::size_t, N, allocator_type>>);
        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_bit_small_vector<std::size_t, N, allocator_type>>);
        static_assert(not std::is_nothrow_move_assignable_v<xstd::basic_small_bitset<std::size_t, N, allocator_type>>);
        BOOST_CHECK(true);
}

// A storage the library does not ship meets the storage contract as it comes: no trait, no wrapper.
BOOST_AUTO_TEST_CASE(AStaticVectorIsAStorageTheSetReadingTakes)
{
        static_assert(xstd::owned_bit_storage<small_words> and xstd::resizable_bit_storage<small_words>);
        static_assert(test::set::bit_set<xstd::bits::detail::set_adaptor<xstd::bits::detail::contiguous_bit_container<small_words>>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
