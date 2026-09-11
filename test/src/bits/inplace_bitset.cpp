//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR, has_inplace_vector
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#ifdef TEST_HAS_INPLACE_VECTOR
#include <xstd/bits/bitset_adaptor.hpp>              // bitset_adaptor
#include <xstd/bits/inplace_bitset.hpp>              // basic_inplace_bitset, inplace_bitset
#include <xstd/bits/detail/block_inplace_vector.hpp> // block_inplace_vector
#include <xstd/bits/bit_set_view.hpp>                // bit_set_view
#include <concepts>                                  // regular, same_as, totally_ordered
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint8_t
#include <new>                                       // bad_alloc
#include <string>                                    // string
#include <utility>                                   // declval
#endif

BOOST_AUTO_TEST_SUITE(InplaceBitset)

#ifdef TEST_HAS_INPLACE_VECTOR

// A capacity of three whole blocks, so a resize can straddle a boundary and still stop short of the capacity.
using T = xstd::basic_inplace_bitset<std::uint8_t, 24>;

// The bitset reading over a run-time width under a compile-time capacity, an alias and nothing more. [design.md#the-public-names]
BOOST_AUTO_TEST_CASE(TheInplaceBitsetIsTheBitsetAdaptorOverAnInplaceVectorOfBlocks)
{
        static_assert(std::same_as<T, xstd::bitset_adaptor<xstd::detail::bits::block_inplace_vector<std::uint8_t, 24>>>);
        static_assert(std::same_as<xstd::inplace_bitset<24>, xstd::basic_inplace_bitset<std::size_t, 24>>);
        static_assert(std::regular<T>);
}

// Two orderings at every width: its own is the bit string's, boost's, and the set reading's is reached through the view. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE(OrderedInfixAndThroughTheView)
{
        static_assert(std::totally_ordered<T>);
        static_assert(std::totally_ordered<decltype(xstd::bit_set_view(std::declval<T&>()))>);
}

// boost::dynamic_bitset's growth, over storage that never allocates: the width moves, the capacity does not. [design.md#a-strict-extension]
BOOST_AUTO_TEST_CASE(ItIsBoostsBitsetAtARunTimeWidthUnderAStaticCapacity)
{
        auto b = T();
        BOOST_CHECK(b.empty());
        BOOST_CHECK_EQUAL(b.max_size(), 24UZ);

        b.resize(9);
        BOOST_CHECK_EQUAL(b.size(), 9UZ);
        BOOST_CHECK_EQUAL(b.capacity(), 24UZ);
        BOOST_CHECK(b.none());

        b.set(2);
        b.set(8);
        BOOST_CHECK_EQUAL(b.count(), 2UZ);
        BOOST_CHECK(b.test(2) and b.test(8));
        BOOST_CHECK_EQUAL(b.to_string(), std::string("100000100"));

        b.flip();
        BOOST_CHECK_EQUAL(b.count(), 7UZ);
        b.reset();
        BOOST_CHECK(b.none());

        b.push_back(true);
        BOOST_CHECK_EQUAL(b.size(), 10UZ);
        BOOST_CHECK(b.test(9));
}

// Past the capacity the storage throws, as [inplace.vector] specifies, and the bitset forwards that unchanged. [design.md#growth]
BOOST_AUTO_TEST_CASE(GrowingPastTheCapacityThrowsBadAlloc)
{
        auto b = T();
        b.resize(24, true);
        BOOST_CHECK(b.all());
        BOOST_CHECK_EQUAL(b.count(), 24UZ);

        BOOST_CHECK_THROW(b.resize(25),    std::bad_alloc);
        BOOST_CHECK_THROW(b.push_back(true), std::bad_alloc);

        BOOST_CHECK_EQUAL(b.size(), 24UZ);
        BOOST_CHECK(b.all());
}

#else

// The column is its storage's: without std::inplace_vector there is no name to test, and saying so keeps the source from being empty. [design.md#the-inplace-column]
BOOST_AUTO_TEST_CASE(TheColumnIsAbsentWithItsStorage)
{
        static_assert(not test::has_inplace_vector);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
