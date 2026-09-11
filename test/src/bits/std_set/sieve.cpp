//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <fmt/format.h>                 // format
#include <fmt/ranges.h>                 // IWYU pragma: keep; the range formatters
#include <opt/set/sieve.hpp>            // filter_twins, generate_candidates, incremental_sieve, sift_primes0, sift_primes1, sift_primes_incremental, sift_primes_segmented
#include <cstddef>                      // size_t
#include <set>                          // set
#include <tuple>                        // tuple
#include <vector>                       // vector

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

// The three sieves are one function of n, and the two unbounded ones earn their place by agreeing with the bounded
// one rather than by being described as equivalent. Every bound below, degenerate ones included, and two window
// widths, so a window shorter than the tail is exercised beside one that swallows it. [design.md#the-unbounded-sieves]
// The bounds stop at N because one of the containers under test is N bits wide: a fixed width is a capacity, and
// asking it for the candidates below N + 1 is asking it to hold N. [design.md#width-is-capacity]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheUnboundedSievesAgreeWithTheBoundedOne, T, Types)
{
        for (auto const n : {0UZ, 1UZ, 2UZ, 3UZ, 4UZ, 5UZ, 9UZ, 10UZ, N / 2UZ, N}) {
                auto const bounded = xstd::sift_primes1<T>(n);
                BOOST_CHECK(xstd::sift_primes_incremental<T>(n) == bounded);
                BOOST_CHECK((xstd::sift_primes_segmented<T, xstd::bit_static_set<8>>(n)) == bounded);
                BOOST_CHECK((xstd::sift_primes_segmented<T, xstd::bit_static_set<256>>(n)) == bounded);
        }
}

// The point of the incremental sieve is that it has no bound at all: it is asked for primes it was never sized for.
BOOST_AUTO_TEST_CASE(TheIncrementalSieveGeneratesWithoutABound)
{
        auto sieve = xstd::incremental_sieve();
        auto first = std::vector<std::size_t>();
        for (auto i = 0UZ; i < 25UZ; ++i) {
                first.push_back(sieve.next());
        }
        // Compared as a range rather than through fmt::format, which the cases above use: a std::vector formatter is
        // a different arm of fmt/ranges.h than the set formatters, and MSVC finds unreachable code inside it that
        // /WX turns into an error. Comparing the values is also the more direct assertion.
        auto const expected = std::vector<std::size_t>{
                2UZ, 3UZ, 5UZ, 7UZ, 11UZ, 13UZ, 17UZ, 19UZ, 23UZ, 29UZ, 31UZ, 37UZ, 41UZ,
                43UZ, 47UZ, 53UZ, 59UZ, 61UZ, 67UZ, 71UZ, 73UZ, 79UZ, 83UZ, 89UZ, 97UZ,
        };
        BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(), first.end(), expected.begin(), expected.end());

        // And it keeps going past where the bounded sieve was asked to stop.
        BOOST_CHECK_EQUAL(sieve.next(), 101UZ);
}

// The segmented sieve sizes its base pass with isqrt, which carries no n < 2 guard because the Newton loop is
// already total there. That is asserted rather than argued: both degenerate inputs, both sides of every perfect
// square up to a point, and the widths where a rounding error would show. [design.md#the-unbounded-sieves]
BOOST_AUTO_TEST_CASE(TheIntegerSquareRootIsExactAndTotal)
{
        BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt(0UZ), 0UZ);
        BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt(1UZ), 1UZ);

        // r * r <= n < (r + 1) * (r + 1) is the whole contract, checked either side of each square.
        for (auto r = 1UZ; r <= 100UZ; ++r) {
                BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt(r * r), r);
                BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt((r * r) - 1UZ), r - 1UZ);
                BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt((r * r) + 1UZ), r);
        }

        // And at widths where a floating-point isqrt would start rounding the wrong way.
        BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt(1UZ << 20UZ), 1UZ << 10UZ);
        BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt((1UZ << 52UZ) - 1UZ), (1UZ << 26UZ) - 1UZ);
        BOOST_CHECK_EQUAL(xstd::detail::sieve::isqrt(1UZ << 52UZ), 1UZ << 26UZ);
}

// generate_candidates is total in n: below two there is nothing to sift, which is an answer rather than a broken
// precondition on iota. [design.md#the-unbounded-sieves]
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSieveIsTotalBelowTwo, T, Types)
{
        BOOST_CHECK(xstd::sift_primes0<T>(0UZ).empty());
        BOOST_CHECK(xstd::sift_primes1<T>(1UZ).empty());
        BOOST_CHECK(xstd::generate_candidates<T>(0UZ).empty());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
