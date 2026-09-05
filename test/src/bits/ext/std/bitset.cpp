//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <xstd/bits/bit_traits.hpp>           // bit_storage, bit_traits, static_bit_extent
#include <xstd/bits/bitset.hpp>               // bitset
#include <xstd/bits/ext/std/bitset.hpp>       // bit_extent, bit_traits, set_find, sequence_find
#include <xstd/bits/ranges/sequence_view.hpp> // sequence_range
#include <xstd/bits/ranges/set_view.hpp>      // set_range, set_view
#include <bitset>                             // bitset
#include <concepts>                           // regular, totally_ordered
#include <ranges>                             // range
#include <tuple>                              // tuple
#include <type_traits>                        // is_nothrow_*, is_trivially_*

BOOST_AUTO_TEST_SUITE(Ext)
BOOST_AUTO_TEST_SUITE(Std)
BOOST_AUTO_TEST_SUITE(Bitset)

// std::bitset takes no Block, so the graded extents do not apply: these are the widths a packed implementation would divide differently.
using Types = std::tuple
<       std::bitset<  0>
,       std::bitset<  1>
,       std::bitset< 64>
,       std::bitset<128>
,       std::bitset<192>
,       std::bitset<256>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(IsRegular, T, Types)
{
        static_assert(std::regular<T>);
}

// One specialization where five stood, and every entry the readings will ask for. [design.md#the-door]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDoorAdaptsIt, T, Types)
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        static_assert(xstd::bit_storage<T>);
        static_assert(xstd::static_bit_extent<T>);
        static_assert(N == T().size());

        auto c = T();
        BOOST_CHECK_EQUAL(traits::size(c), N);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);

        for (auto i = 0UZ; i < N; ++i) {
                traits::insert(c, i);
                BOOST_CHECK(traits::at(c, i));
                BOOST_CHECK_EQUAL(traits::count(c), 1UZ);
                // The synthesized count too, so its per-position ternary sees both a set and a clear one.
                BOOST_CHECK_EQUAL(xstd::detail::bits::scan_count<traits>(c), 1UZ);
                BOOST_CHECK_EQUAL(xstd::detail::bits::scan_first<traits>(c), i);
                BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, N), i);
                BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next<traits>(c, i), N);

                traits::assign(c, i, false);
                BOOST_CHECK(not traits::at(c, i));
        }

        // fill both ways, which is what the set reading's clear() and the sequence reading's fill() become.
        traits::fill(c, true);
        BOOST_CHECK_EQUAL(traits::count(c), N);
        traits::fill(c, false);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);
}

// The element-wise tier's not-found and boundary arms, which only a type without block access reaches. [design.md#per-instantiation-slots]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheWalksAnswerAtTheBoundaries, T, Types)
{
        using traits = xstd::bit_traits<T>;
        constexpr auto N = traits::extent;

        auto const empty = T();
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_count<traits>(empty), 0UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_last <traits>(empty), N);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_first<traits>(empty), N);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(empty, N), N);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(empty, 0UZ), N);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next <traits>(empty, N), N);
}

// The two reserved names are complementary: libstdc++ has _Find_first and no reachable _Getword, MSVC the reverse, libc++ neither. [design.md#the-two-reserved-names]
BOOST_AUTO_TEST_CASE(WhicheverTierThePlatformOffersIsTheOneTaken)
{
        using T = std::bitset<64>;
        using traits = xstd::bit_traits<T>;

        // Whatever this platform supplies, the walks answer the same thing; nothing below is conditioned on which tier ran.
        auto c = T();
        c.set(7);
        c.set(40);

        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_first<traits>(c), 7UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next<traits>(c, 7UZ), 40UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next<traits>(c, 40UZ), 64UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 64UZ), 40UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 40UZ), 7UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 7UZ), 64UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_last<traits>(c), 64UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_count<traits>(c), 2UZ);

        // From the last position, so the inclusive primitive is asked for one past the width. [design.md#inclusive-is-the-primitive]
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next<traits>(c, 63UZ), 64UZ);
}

// The adaptor's whole job: std::bitset has no ordering, and none can legally be added to namespace std, so the set reading supplies it.
BOOST_AUTO_TEST_CASE_TEMPLATE(OrderedThroughTheViewRatherThanInfix, T, Types)
{
        static_assert(not std::totally_ordered<T>);
        static_assert(    std::totally_ordered<xstd::set_view<T>>);
}

// Both readings are reachable, which is what bit_extent, set_find and sequence_find are specialized here for.
BOOST_AUTO_TEST_CASE_TEMPLATE(BothReadingsAreReachable, T, Types)
{
        static_assert(not std::ranges::range<T>);
        static_assert(xstd::ranges::set_range<T>);
        static_assert(xstd::ranges::sequence_range<T>);
}

// Ours answers the same type-trait battery at the same widths, so nothing is given up by building it over the packed array.
BOOST_AUTO_TEST_CASE(OursReproducesItsTraits)
{
        using theirs = std::bitset<64>;
        using ours   = xstd::bitset<64>;

        static_assert(std::regular<theirs>       == std::regular<ours>);
        static_assert(std::totally_ordered<theirs> == std::totally_ordered<ours>);

        static_assert(std::is_nothrow_default_constructible_v<theirs> == std::is_nothrow_default_constructible_v<ours>);
        static_assert(std::is_nothrow_copy_constructible_v<theirs>    == std::is_nothrow_copy_constructible_v<ours>);
        static_assert(std::is_nothrow_move_assignable_v<theirs>       == std::is_nothrow_move_assignable_v<ours>);

        static_assert(std::is_trivially_destructible_v<theirs>       == std::is_trivially_destructible_v<ours>);
        static_assert(std::is_trivially_copy_constructible_v<theirs> == std::is_trivially_copy_constructible_v<ours>);
        static_assert(std::is_trivially_copy_assignable_v<theirs>    == std::is_trivially_copy_assignable_v<ours>);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
