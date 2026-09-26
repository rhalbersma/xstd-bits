//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR, has_inplace_vector
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#ifdef TEST_HAS_INPLACE_VECTOR

#include <xstd/bits/bit_set_view.hpp>                    // bit_set_view
#include <xstd/bits/bounded_bitset.hpp>                  // aligned, basic_bounded_bitset, bounded_bitset
#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t
#include <concepts>                                      // regular, same_as, totally_ordered
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <inplace_vector>                                // inplace_vector
#include <new>                                           // bad_alloc
#include <string>                                        // string
#include <utility>                                       // declval

#endif

BOOST_AUTO_TEST_SUITE(BoundedBitset)

#ifdef TEST_HAS_INPLACE_VECTOR

// A capacity of three whole blocks, so a resize can straddle a boundary and still stop short of the capacity.
using T = xstd::basic_bounded_bitset<std::uint8_t, 24>;

// The bitset reading over a run-time width under a compile-time capacity, adding no member of its own.
BOOST_AUTO_TEST_CASE(TheBoundedBitsetIsTheBitsetAdaptorOverAnInplaceVectorOfBlocks)
{
        static_assert(std::derived_from<T, xstd::bits::detail::bitset_adaptor<xstd::bits::detail::contiguous_bit_container<std::inplace_vector<std::uint8_t, 3>, 24>, T>>);
        static_assert(std::same_as<xstd::bounded_bitset<24>, xstd::basic_bounded_bitset<std::size_t, 24>>);
        static_assert(std::regular<T>);
}

// Two orderings at every width: its own is the bit string's, and the set reading's is reached through the view.
BOOST_AUTO_TEST_CASE(OrderedInfixAndThroughTheView)
{
        static_assert(std::totally_ordered<T>);
        static_assert(std::totally_ordered<decltype(xstd::bit_set_view(std::declval<T&>()))>);
}

// boost::dynamic_bitset's growth, over storage that never allocates: the width moves, the capacity does not.
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

// Past the capacity growth throws bad_alloc, as [inplace.vector] specifies, and leaves the bitset as it was.
BOOST_AUTO_TEST_CASE(GrowingPastTheCapacityThrowsBadAlloc)
{
        auto b = T();
        b.resize(24, true);
        BOOST_CHECK(b.all());
        BOOST_CHECK_EQUAL(b.count(), 24UZ);

        BOOST_CHECK_THROW(b.resize(25), std::bad_alloc);
        BOOST_CHECK_THROW(b.push_back(true), std::bad_alloc);

        BOOST_CHECK_EQUAL(b.size(), 24UZ);
        BOOST_CHECK(b.all());

        // A refused growth is not a partial one: the blocks are asked for before the bits above the width are written.
        auto c = T();
        c.resize(9);
        c.set(2);
        BOOST_CHECK_THROW(c.resize(25, true), std::bad_alloc);
        BOOST_CHECK_EQUAL(c.size(), 9UZ);
        c.resize(20);
        BOOST_CHECK_EQUAL(c.size(), 20UZ);
        BOOST_CHECK_EQUAL(c.count(), 1UZ);
        BOOST_CHECK(c.test(2));
}

// N is the capacity exactly, so a whole block appended where the last block has room left is refused.
BOOST_AUTO_TEST_CASE(TheCapacityIsTheRequestedOneExactly)
{
        using U = xstd::basic_bounded_bitset<std::uint8_t, 12>;
        static_assert(std::same_as<xstd::aligned::basic_bounded_bitset<std::uint8_t, 12>, xstd::basic_bounded_bitset<std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bounded_bitset<std::uint8_t, 16>>, xstd::bits::detail::contiguous_bit_container<std::inplace_vector<std::uint8_t, 2>>>);

        auto b = U();
        BOOST_CHECK_EQUAL(b.max_size(), 12UZ);
        BOOST_CHECK_EQUAL(b.capacity(), 12UZ);
        b.append(std::uint8_t{0xFF});
        BOOST_CHECK_THROW(b.append(std::uint8_t{0xFF}), std::bad_alloc);
        BOOST_CHECK_THROW(b.reserve(13), std::bad_alloc);
        BOOST_CHECK_EQUAL(b.size(), 8UZ);
        BOOST_CHECK(b.all());
}

#else

// The column is its storage's: without std::inplace_vector there is no name to test.
BOOST_AUTO_TEST_CASE(TheColumnIsAbsentWithItsStorage)
{
        static_assert(not test::has_inplace_vector);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
