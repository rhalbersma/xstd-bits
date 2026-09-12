//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>         // graded_extents
#include <xstd/bits/bit_set_view.hpp>   // bit_set_view
#include <xstd/bits/bitset.hpp>         // bitset
#include <xstd/bits/bitset_adaptor.hpp> // swap
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                     // regular, totally_ordered
#include <tuple>                        // tuple_cat
#include <type_traits>                  // is_nothrow_*, is_trivially_*
#include <utility>                      // declval

BOOST_AUTO_TEST_SUITE(Bitset)

// Every Block model within one block, the narrow ones across boundaries, and the widest Block across one too;
// the grading is in test/block_types.hpp. Every case below is a static_assert or one pass over the positions,
// so the three-block instantiations cost what the one-block ones do.
using Types = decltype(std::tuple_cat(
        std::declval<test::graded_extents<xstd::basic_bitset>>(),
        std::declval<test::wide_extents<xstd::basic_bitset>>()));

BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

// Two orderings at every width: its own is the bit string's, boost's, and the set reading's is reached through the view. [design.md#the-ordering-invariant]
BOOST_AUTO_TEST_CASE_TEMPLATE(OrderedInfixAndThroughTheView, T, Types)
{
        static_assert(std::totally_ordered<T>);
        static_assert(std::totally_ordered<decltype(xstd::bit_set_view(std::declval<T&>()))>);
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

// all() and none() read the blocks pairwise -- the whole ones against the last block's mask -- and their two
// arms short-circuit, so each needs a value that stops at the first block and one that runs past it. At the
// wide extents the only callers were set()'s and reset()'s own asserts, which by construction can see the true
// answer alone. One bit short of full and one bit above empty, at every position in turn, asks both arms both
// ways; the pass is linear in the width, which is what this suite spends per type.
BOOST_AUTO_TEST_CASE_TEMPLATE(AllAnyAndNoneReadEveryBlock, T, Types)
{
        auto b = T();
        BOOST_CHECK(     b.none());
        BOOST_CHECK(not  b.any() );
        BOOST_CHECK_EQUAL(b.all(), b.size() == 0);

        b.set();
        BOOST_CHECK(     b.all() );
        BOOST_CHECK_EQUAL(b.any(),  b.size() != 0);
        BOOST_CHECK_EQUAL(b.none(), b.size() == 0);

        for (auto i = 0UZ; i < b.size(); ++i) {
                b.set();
                b.reset(i);
                BOOST_CHECK(not b.all() );
                BOOST_CHECK_EQUAL(b.any(),  b.size() != 1);
                BOOST_CHECK_EQUAL(b.none(), b.size() == 1);

                b.reset();
                b.set(i);
                BOOST_CHECK(    b.any() );
                BOOST_CHECK(not b.none());
                BOOST_CHECK_EQUAL(b.all(), b.size() == 1);
        }
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
