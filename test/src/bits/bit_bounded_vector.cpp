//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                       // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/sequence/dense.hpp>                       // yields_every_position
#include <xstd/bits/bit_bounded_vector.hpp>              // aligned, basic_bit_bounded_vector, bit_bounded_vector
#include <xstd/bits/detail/bounded_blocks.hpp>           // bounded_blocks
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, storage
#include <xstd/bits/detail/sequence_adaptor.hpp>         // sequence_adaptor
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                     // equal
#include <concepts>                                      // same_as
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <limits>                                        // numeric_limits
#include <new>                                           // bad_alloc
#include <ranges>                                        // iota
#include <vector>                                        // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitBoundedVector)

// A capacity of three whole blocks, so the width can straddle a boundary and still stop short of the capacity.
using T = xstd::basic_bit_bounded_vector<std::uint8_t, 24>;

// Dependent, so an absent typedef is a false rather than a hard error.
template<class X>
constexpr bool has_allocator = requires { typename X::allocator_type; }; // NOLINT(readability-redundant-typename): a type-requirement is spelled with typename.

// The sequence reading over a run-time width under a compile-time capacity, built on the sequence adaptor.
BOOST_AUTO_TEST_CASE(TheBoundedSequenceIsTheSequenceAdaptorOverInlineBlocks)
{
        static_assert(std::derived_from<T, xstd::bits::detail::sequence_adaptor<xstd::bits::detail::contiguous_bit_container<xstd::bits::detail::bounded_blocks<std::uint8_t, 3>, 24>, xstd::bits::detail::storage::owned, xstd::bits::detail::window::all, T>>);
        static_assert(std::same_as<xstd::bit_bounded_vector<24>, xstd::basic_bit_bounded_vector<std::size_t, 24>>);
}

// The allocator is the storage's, and this storage has none: the synopsis lines that ask for one do not apply.
BOOST_AUTO_TEST_CASE(ItHasNoAllocatorType)
{
        static_assert(has_allocator<std::vector<bool>>);
        static_assert(not has_allocator<T>);
}

// [inplace.vector.capacity]'s four answer without an object, the capacity being the type's.
BOOST_AUTO_TEST_CASE(TheCapacityIsAPropertyOfTheTypeAndNotOfAnObject)
{
        static_assert(T::capacity() == 24);
        static_assert(T::max_size() == 24);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(T::capacity() == std::inplace_vector<bool, 24>::capacity());

#endif

        // Within the capacity it does nothing, and past it there is nothing to do but refuse.
        T::reserve(T::capacity());
        T::shrink_to_fit();
        BOOST_CHECK_THROW(T::reserve(T::capacity() + 1), std::bad_alloc);
}

// N is the capacity exactly, as std::inplace_vector<bool, N>'s is, and the width moves under it.
BOOST_AUTO_TEST_CASE(TheCapacityIsTheRequestedOneExactly)
{
        static_assert(xstd::basic_bit_bounded_vector<std::uint8_t, 9>::capacity() == 9UZ);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::basic_bit_bounded_vector<std::uint8_t, 9>::capacity() == std::inplace_vector<bool, 9>::capacity());

#endif

        auto v = T();
        BOOST_CHECK_EQUAL(v.capacity(), 24UZ);
        BOOST_CHECK(v.empty());

        // max_size() is the positions there are to hold, which under a static capacity is that capacity.
        BOOST_CHECK_EQUAL(v.max_size(), 24UZ);

        v.resize(17, true);
        BOOST_CHECK_EQUAL(v.size(), 17UZ);
        BOOST_CHECK_EQUAL(v.capacity(), 24UZ);
        BOOST_CHECK(std::ranges::equal(v, std::vector<bool>(17, true)));

        T::reserve(24);
        T::shrink_to_fit();
        BOOST_CHECK_EQUAL(v.size(), 17UZ);
}

// Distinct capacities are distinct types, and the aligned form rounds up to whole blocks as the static column's does.
BOOST_AUTO_TEST_CASE(TheCapacityIsPartOfTheType)
{
        static_assert(not std::same_as<xstd::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::aligned::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::aligned::bit_bounded_vector<9>, xstd::bit_bounded_vector<std::numeric_limits<std::size_t>::digits>>);

        // Named by its storage alone, the container holds every bit of it: one storage, however it is spelled.
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_bounded_vector<std::uint8_t, 16>>, xstd::bits::detail::contiguous_bit_container<xstd::bits::detail::bounded_blocks<std::uint8_t, 2>>>);
}

// Every position, densely, agreeing with the subscript -- and not a contiguous range, which no proxy sequence can be.
BOOST_AUTO_TEST_CASE(ItYieldsEveryPosition)
{
        auto c = T();
        test::sequence::yields_every_position(c);

        for (auto const n : std::views::iota(0UZ, c.size())) {
                c[n] = (n % 3UZ == 0UZ);
        }
        test::sequence::yields_every_position(c);
}

BOOST_AUTO_TEST_SUITE_END()
