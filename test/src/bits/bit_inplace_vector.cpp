//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>              // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <test/inplace_vector.hpp>               // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR, has_inplace_vector
#ifdef TEST_HAS_INPLACE_VECTOR
#include <test/sequence/concepts.hpp>            // bit_sequence, inplace_vector_bool, inplace_vector_bool_ranges
#include <xstd/bits/sequence_adaptor.hpp>        // sequence_adaptor
#include <xstd/bits/bit_inplace_vector.hpp>      // basic_bit_inplace_vector, bit_inplace_vector
#include <xstd/bits/block_sequence.hpp>          // block_inplace_vector
#include <xstd/bits/ownership.hpp>               // ownership
#include <algorithm>                             // equal
#include <concepts>                              // same_as
#include <cstddef>                               // size_t
#include <cstdint>                               // uint8_t
#include <limits>                                // numeric_limits
#include <new>                                   // bad_alloc
#include <ranges>                                // count, iota, to, transform
#include <vector>                                // vector
#endif

BOOST_AUTO_TEST_SUITE(BitInplaceVector)

#ifdef TEST_HAS_INPLACE_VECTOR

// A capacity of three whole blocks, so the width can straddle a boundary and still stop short of the capacity.
using T = xstd::basic_bit_inplace_vector<24, std::uint8_t>;

// Dependent, so an absent typedef is a false rather than a hard error.
template<class X>
constexpr bool has_allocator = requires { typename X::allocator_type; };

// The sequence reading over a run-time width under a compile-time capacity, an alias and nothing more. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheInplaceSequenceIsTheSequenceAdaptorOverAnInplaceVectorOfBlocks)
{
        static_assert(std::same_as<T, xstd::sequence_adaptor<xstd::block_inplace_vector<std::uint8_t, 24>, xstd::ownership::owns, false>>);
        static_assert(std::same_as<xstd::bit_inplace_vector<24>, xstd::basic_bit_inplace_vector<24, std::size_t>>);
        static_assert(test::sequence::bit_sequence<T>);
}

// Every line of [vector.bool] the allocator does not reach, the model first so the checklist is known to be honest. [design.md#the-inplace-column]
BOOST_AUTO_TEST_CASE(ItAnswersEveryLineOfStdVectorBoolButTheAllocator)
{
        static_assert(test::sequence::inplace_vector_bool<std::vector<bool>>);
        static_assert(test::sequence::inplace_vector_bool<T>);
        static_assert(test::sequence::inplace_vector_bool<xstd::bit_inplace_vector<24>>);
        static_assert(test::sequence::inplace_vector_bool_ranges<T>);

        // The allocator is the storage's, and this storage has none: the checklist that asks for one does not apply.
        static_assert(has_allocator<std::vector<bool>>);
        static_assert(not has_allocator<T>);
}

// The blocks are whole, so the capacity is the requested one rounded up, and the width moves under it. [design.md#the-inplace-column]
BOOST_AUTO_TEST_CASE(TheCapacityIsTheRequestedOneRoundedUpToWholeBlocks)
{
        static_assert(xstd::basic_bit_inplace_vector<9, std::uint8_t>().capacity() == 16UZ);

        auto v = T();
        BOOST_CHECK_EQUAL(v.capacity(), 24UZ);
        BOOST_CHECK(v.empty());

        // max_size() is the positions there are to hold, which under a static capacity is that capacity. [design.md#max-size-is-the-bits]
        BOOST_CHECK_EQUAL(v.max_size(), 24UZ);

        v.resize(17, true);
        BOOST_CHECK_EQUAL(v.size(), 17UZ);
        BOOST_CHECK_EQUAL(v.capacity(), 24UZ);
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>(17, true)));

        v.reserve(24);
        v.shrink_to_fit();
        BOOST_CHECK_EQUAL(v.size(), 17UZ);
}

// Past the capacity the storage throws, as [inplace.vector] specifies, and the sequence forwards that unchanged. [design.md#growth]
BOOST_AUTO_TEST_CASE(GrowingPastTheCapacityThrowsBadAlloc)
{
        auto v = std::views::iota(0UZ, 24UZ) | std::views::transform([](auto i) { return i % 2 == 0; }) | std::ranges::to<T>();
        BOOST_CHECK_EQUAL(v.size(), 24UZ);
        BOOST_CHECK_EQUAL(std::ranges::count(v, true), 12);

        BOOST_CHECK_THROW(v.push_back(true), std::bad_alloc);
        BOOST_CHECK_THROW(v.resize(25),      std::bad_alloc);
        BOOST_CHECK_THROW(v.reserve(25),     std::bad_alloc);

        // The failed growth left the value alone, which is what the strong guarantee buys.
        BOOST_CHECK_EQUAL(v.size(), 24UZ);
        BOOST_CHECK_EQUAL(std::ranges::count(v, true), 12);

        v.pop_back();
        v.push_back(true);
        BOOST_CHECK_EQUAL(v.size(), 24UZ);
        BOOST_CHECK(static_cast<bool>(v.back()));
}

#else

// The column is its storage's: without std::inplace_vector there is no name to test, and saying so keeps the source from being empty. [design.md#the-inplace-column]
BOOST_AUTO_TEST_CASE(TheColumnIsAbsentWithItsStorage)
{
        static_assert(not test::has_inplace_vector);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
