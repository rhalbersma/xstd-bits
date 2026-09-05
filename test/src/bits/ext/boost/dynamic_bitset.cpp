//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/dynamic.hpp>                       // dynamic
#include <xstd/bits/bit_traits.hpp>               // bit_storage, bit_traits, block_readable, static_bit_extent
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // the hooks that make dynamic_bitset a bit range
#include <xstd/bits/ranges/sequence_view.hpp>     // sequence_range
#include <xstd/bits/ranges/set_view.hpp>          // set_range, set_view
#include <algorithm>                              // lexicographical_compare
#include <compare>                                // strong_ordering
#include <concepts>                               // regular, totally_ordered
#include <cstddef>                                // size_t
#include <set>                                    // set
#include <type_traits>                            // is_trivially_copyable_v

BOOST_AUTO_TEST_SUITE(Ext)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)

using T = boost::dynamic_bitset<>;

BOOST_AUTO_TEST_CASE(IsRegular)
{
        static_assert(std::regular<T>);
}

// One specialization where three stood; the only adapted type whose width grows, and the only one with no block access. [design.md#the-door]
BOOST_AUTO_TEST_CASE(TheDoorAdaptsIt)
{
        using traits = xstd::bit_traits<T>;

        static_assert(xstd::bit_storage<T>);
        static_assert(not xstd::static_bit_extent<T>);
        static_assert(not xstd::block_readable<traits, T>);

        auto c = T(9);
        BOOST_CHECK_EQUAL(traits::size(c), 9UZ);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);

        // npos becomes size(), the door being total where boost is not. [design.md#total-versus-precondition]
        BOOST_CHECK_EQUAL(traits::find_first(c), 9UZ);

        traits::insert(c, 3);
        BOOST_CHECK(traits::at(c, 3));
        BOOST_CHECK_EQUAL(traits::find_first(c), 3UZ);
        BOOST_CHECK_EQUAL(traits::find_next(c, 3), 9UZ);

        // The one entry that grows: a position past the width extends it rather than asserting.
        traits::insert(c, 20);
        BOOST_CHECK_EQUAL(traits::size(c), 21UZ);
        BOOST_CHECK_EQUAL(traits::count(c), 2UZ);
        BOOST_CHECK_EQUAL(traits::find_next(c, 3), 20UZ);

        // Synthesized, boost having neither: the width answers one and the element walk the other.
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_last<traits>(c), 21UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 21), 20UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 20), 3UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev<traits>(c, 3), 21UZ);

        traits::assign(c, 20, false);
        BOOST_CHECK(not traits::at(c, 20));

        traits::fill(c, true);
        BOOST_CHECK_EQUAL(traits::count(c), 21UZ);
        traits::fill(c, false);
        BOOST_CHECK_EQUAL(traits::count(c), 0UZ);

}

// The element-wise tier's not-found and boundary arms, which only a type without block access reaches. [design.md#per-instantiation-slots]
BOOST_AUTO_TEST_CASE(TheWalksAnswerAtTheBoundaries)
{
        using traits = xstd::bit_traits<T>;

        auto const empty = T(9);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_count<traits>(empty), 0UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_last <traits>(empty), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_first<traits>(empty), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(empty, 9UZ), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(empty, 0UZ), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next <traits>(empty, 9UZ), 9UZ);

        // And its found arms, which the empty case cannot reach; the walks are scored per instantiation. [design.md#per-instantiation-slots]
        auto populated = T(9);
        populated.set(1);
        populated.set(5);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_count<traits>(populated), 2UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_first<traits>(populated), 1UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next <traits>(populated, 1UZ), 5UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next <traits>(populated, 5UZ), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_next <traits>(populated, 8UZ), 9UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(populated, 9UZ), 5UZ);
        BOOST_CHECK_EQUAL(xstd::detail::bits::scan_prev <traits>(populated, 1UZ), 9UZ);
}

// The one bitset that orders itself, and wrongly: over a 4-bit universe its own < disagrees with std::set on 88 of 256 pairs, the view's on none.
BOOST_AUTO_TEST_CASE(TheViewReplacesItsOrderingRatherThanTrustingIt)
{
        static_assert(std::totally_ordered<T>);
        static_assert(std::totally_ordered<xstd::set_view<T>>);

        constexpr auto N = 4UZ;
        auto disagreements = 0;
        for (auto i = 0UZ; i < (1UZ << N); ++i) {
                for (auto j = 0UZ; j < (1UZ << N); ++j) {
                        auto x = T(N);
                        auto y = T(N);
                        auto kx = std::set<std::size_t>();
                        auto ky = std::set<std::size_t>();
                        for (auto k = 0UZ; k < N; ++k) {
                                if ((i >> k & 1UZ) != 0UZ) { x.set(k); kx.insert(k); }
                                if ((j >> k & 1UZ) != 0UZ) { y.set(k); ky.insert(k); }
                        }

                        // What the keys themselves say, which is what the set reading means.
                        auto const expected = std::ranges::lexicographical_compare(kx, ky);

                        BOOST_CHECK_EQUAL((xstd::set_view(x) <=> xstd::set_view(y)) < 0, expected);
                        disagreements += static_cast<int>((x < y) != expected);
                }
        }

        // Not merely different in principle: different on a third of all pairs.
        BOOST_CHECK_EQUAL(disagreements, 88);
}

BOOST_AUTO_TEST_CASE(BothReadingsAreReachable)
{
        static_assert(xstd::ranges::set_range<T>);
        static_assert(xstd::ranges::sequence_range<T>);
}

// It owns its storage, so the extent is a run-time value and the trivial and nothrow batteries do not apply.
BOOST_AUTO_TEST_CASE(ItIsTheDynamicOne)
{
        static_assert(    test::dynamic<T>);
        static_assert(not std::is_trivially_copyable_v<T>);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
