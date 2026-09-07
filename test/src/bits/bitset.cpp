//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/block_types.hpp>          // graded_extents
#include <xstd/bits/basic_bitset.hpp>    // swap
#include <xstd/bits/bitset.hpp>          // bitset
#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <concepts>                      // regular, totally_ordered
#include <type_traits>                   // is_nothrow_*, is_trivially_*
#include <utility>                       // declval

BOOST_AUTO_TEST_SUITE(Bitset)

using Types = test::graded_extents<xstd::bitset>;

BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

// Deliberately not orderable by itself, because std::bitset is not: the ordering is reached through the set reading.
BOOST_AUTO_TEST_CASE_TEMPLATE(OrderedThroughTheViewRatherThanInfix, T, Types)
{
        static_assert(not std::totally_ordered<T>);
        static_assert(    std::totally_ordered<decltype(xstd::bit_set_view(std::declval<T&>()))>);
}

// A fixed-width bitset owns no storage, so every operation on it is nothrow.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsNoThrow, T, Types)
{
        static_assert(std::is_nothrow_destructible_v<T>);
        static_assert(std::is_nothrow_default_constructible_v<T>);
        static_assert(std::is_nothrow_copy_constructible_v<T>);
        static_assert(std::is_nothrow_copy_assignable_v<T>);
        static_assert(std::is_nothrow_move_constructible_v<T>);
        static_assert(std::is_nothrow_move_assignable_v<T>);
}

// Trivial in every respect but default construction, which zeroes the bits.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsTrivial, T, Types)
{
        static_assert(    std::is_trivially_destructible_v<T>);
        static_assert(not std::is_trivially_default_constructible_v<T>);
        static_assert(    std::is_trivially_copy_constructible_v<T>);
        static_assert(    std::is_trivially_copy_assignable_v<T>);
        static_assert(    std::is_trivially_move_constructible_v<T>);
        static_assert(    std::is_trivially_move_assignable_v<T>);
}

// The proxy from bit_span: b[i] = b[j] must move the bit, not the proxy, or swap breaks.
BOOST_AUTO_TEST_CASE(TheMutableSubscriptHandsOutAnAssignableProxy)
{
        auto b = xstd::bitset<8>{};

        b[3] = true;
        BOOST_CHECK(b[3]);
        BOOST_CHECK(b.test(3));
        BOOST_CHECK_EQUAL(b.count(), 1);

        b[3] = false;
        BOOST_CHECK(not b[3]);

        b[1] = true;
        b[2] = b[1];
        BOOST_CHECK(b[1]);
        BOOST_CHECK(b[2]);

        BOOST_CHECK_EQUAL(~b[2], false);
        b[2].flip();
        BOOST_CHECK(not b[2]);

        auto const x = b[1];
        auto const y = b[2];
        swap(x, y);
        BOOST_CHECK(not b[1]);
        BOOST_CHECK(b[2]);

        auto z = false;
        swap(b[2], z);
        BOOST_CHECK(not b[2]);
        BOOST_CHECK(z);

        swap(z, b[2]);
        BOOST_CHECK(b[2]);
        BOOST_CHECK(not z);
}

BOOST_AUTO_TEST_SUITE_END()
