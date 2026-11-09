//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm/bit_mismatch.hpp> // bit_mismatch
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array, bit_array
#include <xstd/bits/bit_span/bit_span.hpp>          // bit_span
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/algorithm/sequences.hpp>             // agrees_through_iterators, for_each_window, patterned
#include <algorithm>                                // mismatch, mismatch_result
#include <concepts>                                 // same_as
#include <cstdint>                                  // uint8_t
#include <ranges>                                   // begin, dangling, iota, iterator_t
#include <utility>                                  // declval
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitMismatch)

namespace {

template<class I>
concept mismatch_between = requires (I i) { xstd::ranges::bit_mismatch(i, i, i, i); xstd::bit_mismatch(i, i, i); xstd::bit_mismatch(i, i, i, i); };

template<class R1, class R2>
concept comparable = requires (R1 const& r1, R2 const& r2) { xstd::ranges::bit_mismatch(r1, r2); };

// Both iterators of the algorithm's answer where std::ranges::mismatch puts them, as offsets from each begin.
template<class R>
[[nodiscard]] constexpr auto agrees(R const& r1, R const& r2)
        -> bool
{
        auto const got  = xstd::ranges::bit_mismatch(r1, r2);
        auto const want = std::ranges::mismatch(r1, r2);
        return got.in1 - std::ranges::begin(r1) == want.in1 - std::ranges::begin(r1) and got.in2 - std::ranges::begin(r2) == want.in2 - std::ranges::begin(r2);
}

} // namespace

// Equal sequences, and one position flipped at every index, across block boundaries and in narrow and wide blocks.
BOOST_AUTO_TEST_CASE(AgreesWithRangesMismatchAtEveryPosition)
{
        auto wrong = 0;
        for (auto const n : {0UZ, 1UZ, 8UZ, 21UZ, 64UZ, 130UZ}) {
                auto const v = test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(n));
                wrong += static_cast<int>(not agrees(v, v));
                for (auto const i : std::views::iota(0UZ, n)) {
                        auto w = v;
                        w[i]   = not w[i];
                        wrong += static_cast<int>(not agrees(v, w)) + static_cast<int>(not agrees(w, v));
                }
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// Two run-time widths stop at the shorter's end, though the longer holds set bits in what is the shorter's padding.
BOOST_AUTO_TEST_CASE(TwoWidthsStopAtTheShortersEnd)
{
        auto wrong = 0;
        for (auto const shorter : {0UZ, 1UZ, 5UZ, 8UZ, 13UZ, 64UZ}) {
                for (auto const longer : {shorter, shorter + 1UZ, shorter + 3UZ, shorter + 70UZ}) {
                        auto const a = xstd::bit_vector(shorter);
                        auto const b = xstd::bit_vector(longer, true);
                        auto const c = xstd::bit_vector(longer);
                        wrong += static_cast<int>(not agrees(a, b)) + static_cast<int>(not agrees(b, a)) + static_cast<int>(not agrees(a, c)) + static_cast<int>(not agrees(c, a));
                }
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// Windows compare a bool at a time, at every offset and length, against a whole view of the same width.
BOOST_AUTO_TEST_CASE(WindowsCompareAsRangesMismatch)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto b       = a;
        b[13]        = not b[13];
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void {
                auto const v = xstd::bit_span(b).subspan(0UZ, w.size());
                wrong += static_cast<int>(not agrees(w, decltype(w)(v)));
        });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// An rvalue owner's iterator dangles, as std::ranges::mismatch's does; two different types do not compare.
BOOST_AUTO_TEST_CASE(AnswersAsRangesMismatchDoes)
{
        using A = xstd::bit_array<8>;
        static_assert(std::same_as<decltype(xstd::ranges::bit_mismatch(A(), std::declval<A&>())), std::ranges::mismatch_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>()), xstd::bit_array<70>()));
        static_assert(comparable<A, A> and comparable<A, xstd::bit_array<9>> and comparable<A, xstd::bit_vector> and not comparable<std::vector<bool>, std::vector<bool>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// Between two pairs of iterators, as std::ranges::mismatch and as std::mismatch, against another sequence one bool apart.
BOOST_AUTO_TEST_CASE(MismatchesBetweenTwoPairsOfIterators)
{
        auto const agrees = [](auto first, auto last, auto const& w) -> bool {
                auto other = xstd::bit_vector(first, last);
                if (not other.empty()) {
                        other[other.size() / 2UZ] = not other[other.size() / 2UZ];
                }
                auto const want   = xstd::ranges::bit_mismatch(w, other).in1 - std::ranges::begin(w);
                auto const ranges = xstd::ranges::bit_mismatch(first, last, other.begin(), other.end());
                auto const pair3  = xstd::bit_mismatch(first, last, other.begin());
                auto const pair4  = xstd::bit_mismatch(first, last, other.begin(), other.end());
                return ranges.in1 - first == want and ranges.in2 - other.begin() == want and pair3.first - first == want and pair4.second - other.begin() == want;
        };
        BOOST_CHECK_EQUAL(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), agrees), 0);
        static_assert(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 13>()), agrees) == 0);
        static_assert(mismatch_between<std::ranges::iterator_t<xstd::bit_array<8> const>> and not mismatch_between<std::vector<bool>::const_iterator>);
}

BOOST_AUTO_TEST_SUITE_END()
