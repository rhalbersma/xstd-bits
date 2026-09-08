//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <fmt/format.h>                 // format
#include <fmt/ranges.h>                 // IWYU pragma: keep; the range formatters
#include <opt/set/sieve.hpp>            // filter_twins, sift_primes0, sift_primes1
#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <cstddef>                      // size_t
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(Sieve)

inline constexpr auto N = 100UZ;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::bit_static_set<N>
,       xstd::bit_set
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSiftedPrimesAndTwinsFormatAsExpected, T, Types)
{
        auto const primes0 = xstd::sift_primes0<T>(N);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", primes0),
                "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}"
        );

        auto const primes1 = xstd::sift_primes1<T>(N);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", primes1),
                "{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97}"
        );

        auto const twins = xstd::filter_twins(primes1);
        BOOST_CHECK_EQUAL(
                fmt::format("{}", twins),
                "{3, 5, 7, 11, 13, 17, 19, 29, 31, 41, 43, 59, 61, 71, 73}"
        );
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SievesTooSmallForTheSquareBreakStillSiftCorrectly, T, Types)
{
        // Below three candidates sift_primes1 runs to exhaustion, and filter_twins returns before it has a triple.
        auto const none = xstd::sift_primes1<T>(2);
        BOOST_CHECK(none.empty());
        BOOST_CHECK(xstd::filter_twins(none).empty());

        auto const one = xstd::sift_primes1<T>(3);
        BOOST_CHECK_EQUAL(fmt::format("{}", one), "{2}");
        BOOST_CHECK(xstd::filter_twins(one).empty());
}

// The README offers a word-at-a-time twins as the dense container's answer to the elementwise one; the two agreeing
// is the whole claim, so it is asserted rather than described. [design.md#the-sieve]
BOOST_AUTO_TEST_CASE(TheDataParallelTwinsAgreeWithTheElementwiseOnes)
{
        auto const primes = xstd::sift_primes1<xstd::bit_static_set<N>>(N);
        auto const elementwise = xstd::filter_twins(primes);
        auto const parallel = primes & (primes << 2 | primes >> 2);
        BOOST_CHECK(elementwise == parallel);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
