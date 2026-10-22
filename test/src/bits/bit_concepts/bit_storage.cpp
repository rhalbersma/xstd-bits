//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bit_concepts/bit_storage.hpp> // bit_storage
#include <xstd/bits/bit_vector.hpp>               // basic_bit_vector, bit_vector
#include <boost/container/small_vector.hpp>       // small_vector
#include <boost/container/static_vector.hpp>      // static_vector
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                  // array
#include <cstdint>                                // uint16_t, uint32_t, uint64_t, uint8_t
#include <deque>                                  // deque
#include <memory_resource>                        // monotonic_buffer_resource
#include <span>                                   // span
#include <vector>                                 // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitStorage)

// The sequences of blocks a user hands an owner: on the heap, in an arena, inline, or inline then on the heap.
BOOST_AUTO_TEST_CASE(ContiguousResizableSequencesOfBlocksAreBitStorage)
{
        static_assert(xstd::bit_storage<std::vector<std::uint64_t>> and xstd::bit_storage<std::vector<std::uint8_t>>);
        static_assert(xstd::bit_storage<std::pmr::vector<std::uint32_t>>);
        static_assert(xstd::bit_storage<boost::container::static_vector<std::uint16_t, 4>>);
        static_assert(xstd::bit_storage<boost::container::small_vector<std::uint64_t, 2>>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::bit_storage<std::inplace_vector<std::uint64_t, 2>>);
#endif
        BOOST_CHECK(true);
}

// An arena's vector is the storage of an owner like any other, its blocks allocated from the arena.
BOOST_AUTO_TEST_CASE(AnArenaHoldsTheBlocks)
{
        auto arena = std::pmr::monotonic_buffer_resource();
        auto v     = xstd::basic_bit_vector<std::pmr::vector<std::uint64_t>>(130, true, &arena);
        BOOST_CHECK(v.size() == 130UZ);
        BOOST_CHECK(v.get_allocator().resource() == &arena);
}

// Bools rather than blocks, signed blocks, a fixed count, blocks out of order in memory, and blocks not owned.
BOOST_AUTO_TEST_CASE(TheNearMissesAreNot)
{
        static_assert(not xstd::bit_storage<std::vector<bool>> and not xstd::bit_storage<std::vector<int>>);
        static_assert(not xstd::bit_storage<std::array<std::uint64_t, 2>>);
        static_assert(not xstd::bit_storage<std::deque<std::uint64_t>>);
        static_assert(not xstd::bit_storage<std::span<std::uint64_t>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
