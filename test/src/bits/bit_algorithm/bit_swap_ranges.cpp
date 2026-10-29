//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                // bools_of, for_each_sequence, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>                      // for_each_type
#include <xstd/bits/bit_algorithm/bit_swap_ranges.hpp> // bit_swap_ranges
#include <xstd/bits/bit_array.hpp>                     // bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>         // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>             // bit_span
#include <xstd/bits/bit_vector.hpp>                    // bit_vector
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                   // swap_ranges, swap_ranges_result
#include <concepts>                                    // same_as
#include <cstddef>                                     // ptrdiff_t, size_t
#include <cstdint>                                     // uint64_t, uint8_t
#include <ranges>                                      // begin, dangling, iterator_t, subrange
#include <utility>                                     // declval, forward
#include <vector>                                      // vector

BOOST_AUTO_TEST_SUITE(BitSwapRanges)

namespace {

template<class R1, class R2>
concept swappable = requires (R1&& r1, R2&& r2) { xstd::ranges::bit_swap_ranges(std::forward<R1>(r1), std::forward<R2>(r2)); };

// The algorithm and std::ranges::swap_ranges between x and a window of a striped sequence over Block.
template<class Block, class X>
[[nodiscard]] auto agrees(X x, std::size_t offset, std::size_t length)
        -> bool
{
        auto y          = test::algorithm::striped_vector<Block>(offset + length + 5UZ);
        auto xm         = test::algorithm::bools_of(x);
        auto ym         = test::algorithm::bools_of(y);
        auto const d    = static_cast<std::ptrdiff_t>(offset);
        auto const want = std::ranges::swap_ranges(xm, std::ranges::subrange(ym.begin() + d, ym.begin() + d + static_cast<std::ptrdiff_t>(length)));
        auto const w    = xstd::bit_span(y).subspan(offset, length);
        auto const got  = xstd::ranges::bit_swap_ranges(x, w);
        return got.in1 - x.begin() == want.in1 - xm.begin() and got.in2 - w.begin() == want.in2 - (ym.begin() + d) and test::algorithm::bools_of(x) == xm and test::algorithm::bools_of(y) == ym;
}

// Against a shorter, an equal and a longer window, of either block width at every offset.
template<class X>
[[nodiscard]] auto disagreements(X const& x)
        -> int
{
        auto const n = x.size();
        auto wrong   = 0;
        for (auto const offset : test::algorithm::offsets) {
                for (auto const length : {n / 2UZ, n, n + 9UZ}) {
                        wrong += static_cast<int>(not agrees<std::uint8_t>(x, offset, length)) + static_cast<int>(not agrees<std::uint64_t>(x, offset, length));
                }
        }
        return wrong;
}

} // namespace

// Every input of every column of ours, against windows over two block widths at aligned and unaligned offsets.
BOOST_AUTO_TEST_CASE(AgreesWithRangesSwapRangesOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// In a constant expression, between writable bools of ours alone.
BOOST_AUTO_TEST_CASE(SwapsOurWritableSequencesAlone)
{
        static_assert([] -> bool {
                auto a       = test::algorithm::patterned(xstd::bit_array<70>());
                auto b       = xstd::bit_array<70>();
                auto const r = xstd::ranges::bit_swap_ranges(a, b);
                return r.in1 == a.end() and r.in2 == b.end() and test::algorithm::bools_of(b) == test::algorithm::bools_of(test::algorithm::patterned(xstd::bit_array<70>())) and test::algorithm::bools_of(a) == test::algorithm::bools_of(xstd::bit_array<70>());
        }());
        using A = xstd::bit_array<8>;
        static_assert(swappable<A&, xstd::bit_vector&> and swappable<A&, decltype(xstd::bit_span(std::declval<A&>()).subspan(1UZ, 3UZ))>);
        static_assert(not swappable<A const&, A&> and not swappable<A&, std::vector<bool>&> and not swappable<A&, xstd::bit_fixed_set<8>&>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_swap_ranges(A(), std::declval<A&>())), std::ranges::swap_ranges_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
