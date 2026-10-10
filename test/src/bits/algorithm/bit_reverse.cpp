//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>        // patterned
#include <xstd/bits/algorithm/bit_reverse.hpp> // bit_reverse
#include <xstd/bits/bit_array.hpp>             // basic_bit_array, bit_array
#include <xstd/bits/bit_span.hpp>              // bit_span
#include <xstd/bits/bit_vector.hpp>            // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                           // count, equal, reverse
#include <concepts>                            // same_as
#include <cstdint>                             // uint8_t
#include <ranges>                              // dangling, end, iterator_t
#include <utility>                             // declval, forward
#include <vector>                              // vector

BOOST_AUTO_TEST_SUITE(BitReverse)

namespace {

template<class R>
concept reversible = requires (R&& r) { xstd::bit_reverse(std::forward<R>(r)); };

// The algorithm on s against std::ranges::reverse on the same bools, and the end iterator it answers.
template<class S>
[[nodiscard]] constexpr auto agrees(S s)
        -> bool
{
        auto model = std::vector<bool>(s.begin(), s.end());
        std::ranges::reverse(model);
        auto const last = xstd::bit_reverse(s);
        return std::ranges::equal(s, model) and last == std::ranges::end(s);
}

} // namespace

// Owners at widths that fill their last block, fall short of it, and fill none, in narrow and wide blocks.
BOOST_AUTO_TEST_CASE(ReversesAsRangesReverse)
{
        static_assert(agrees(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>())));
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<64>())));
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<130>())));
        static_assert(agrees(xstd::bit_array<0>()));
        auto wrong = 0;
        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 63UZ, 64UZ, 65UZ, 200UZ}) {
                wrong += static_cast<int>(not agrees(test::algorithm::patterned(xstd::bit_vector(n))));
                wrong += static_cast<int>(not agrees(test::algorithm::patterned(xstd::basic_bit_vector<std::uint8_t>(n))));
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A whole view reverses the bits it views, and the padding stays clear for the owner's count to see.
BOOST_AUTO_TEST_CASE(AWholeViewReversesWhatItViews)
{
        auto a          = xstd::basic_bit_array<std::uint8_t, 13>();
        a[0]            = true;
        auto const last = xstd::bit_reverse(xstd::bit_span(a));
        BOOST_CHECK(a[12] and std::ranges::count(a, true) == 1);
        static_assert(std::same_as<decltype(last), std::ranges::iterator_t<decltype(xstd::bit_span(a))> const>);
}

// Not a window, which shares its end blocks, nor anything const, and an rvalue owner's end dangles as in std::ranges.
BOOST_AUTO_TEST_CASE(WhatItReverses)
{
        using A = xstd::bit_array<8>;
        static_assert(reversible<A&> and reversible<xstd::bit_vector&> and reversible<decltype(xstd::bit_span(std::declval<A&>()))>);
        static_assert(not reversible<A const&> and not reversible<decltype(xstd::bit_span(std::declval<A const&>()))>);
        static_assert(not reversible<decltype(xstd::bit_span(std::declval<A&>()).subspan(1UZ, 4UZ))>);
        static_assert(not reversible<std::vector<bool>&>);
        static_assert(std::same_as<decltype(xstd::bit_reverse(A())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
