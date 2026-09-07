//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE
#include <fmt/format.h>                           // format
#include <fmt/ranges.h>                           // IWYU pragma: keep; the range formatters
#include <opt/bitset/sieve.hpp>                   // filter_twins, sift_primes0, sift_primes1
#include <xstd/bits/bit_static_set.hpp>           // bit_static_set
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <xstd/bits/ext/xstd/bitset.hpp>          // bitset
#include <xstd/bits/bit_set_view.hpp>          // bit_set_view
#include <bitset>                                 // bitset
#include <cstddef>                                // size_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(Sieve)

inline constexpr auto N = 100UZ;

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset<N>
,        xstd::bitset<N>
,        xstd::bit_static_set<N>
,        xstd::basic_dynamic_bitset<std::size_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSiftedPrimesAndTwinsFormatAsExpected, T, Types)
{
        auto const primes0 = xstd::sift_primes0<T>(N);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", xstd::bit_set_view(primes0)),
                "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}")
        ;

        auto const primes1 = xstd::sift_primes1<T>(N);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", xstd::bit_set_view(primes1)),
                "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}")
        ;

        auto const twins = xstd::filter_twins(primes1);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", xstd::bit_set_view(twins)),
                "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}"
        );
}

BOOST_AUTO_TEST_CASE(ATwoBitSieveSiftsToNothing)
{
        // A two-bit sieve leaves nothing, so sift_primes1 runs its loop to exhaustion; only dynamic_bitset can be that small.
        BOOST_CHECK(xstd::sift_primes1<boost::dynamic_bitset<>>(2).none());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
