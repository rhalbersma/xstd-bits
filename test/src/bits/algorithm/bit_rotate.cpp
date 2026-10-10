//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>       // patterned
#include <xstd/bits/algorithm/bit_rotate.hpp> // bit_rotate
#include <xstd/bits/bit_array.hpp>            // basic_bit_array, bit_array
#include <xstd/bits/bit_span.hpp>             // bit_span
#include <xstd/bits/bit_vector.hpp>           // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                          // count, equal, rotate
#include <concepts>                           // same_as
#include <cstddef>                            // ptrdiff_t, size_t
#include <cstdint>                            // uint8_t
#include <ranges>                             // begin, dangling, end, iota, iterator_t, next, size
#include <utility>                            // declval, forward
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(BitRotate)

namespace {

template<class R>
concept rotatable = requires (R&& r) { xstd::bit_rotate(std::forward<R>(r), std::ranges::begin(r)); };

// The algorithm on s against std::rotate on the same bools as ints, and the subrange it answers, at one turn.
template<class S>
[[nodiscard]] constexpr auto agrees(S s, std::size_t turn)
        -> bool
{
        auto model   = std::vector<int>(s.begin(), s.end());
        auto const t = static_cast<std::ptrdiff_t>(turn);
        std::ranges::rotate(model, model.begin() + t);
        auto const [first, last] = xstd::bit_rotate(s, std::ranges::next(std::ranges::begin(s), t));
        auto const as_int        = [](bool b) noexcept -> int { return static_cast<int>(b); };
        return std::ranges::equal(s, model, {}, as_int) and first == std::ranges::end(s) - t and last == std::ranges::end(s);
}

// Every turn in [0, size()] of s.
template<class S>
[[nodiscard]] constexpr auto disagreements(S const& s)
        -> int
{
        auto wrong = 0;
        for (auto const turn : std::views::iota(0UZ, std::ranges::size(s) + 1UZ)) {
                wrong += static_cast<int>(not agrees(s, turn));
        }
        return wrong;
}

} // namespace

// Every turn of owners at widths that fill their last block, fall short of it, and fill none.
BOOST_AUTO_TEST_CASE(RotatesAsRangesRotate)
{
        static_assert(disagreements(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>())) == 0);
        static_assert(disagreements(test::algorithm::patterned(xstd::bit_array<64>())) == 0);
        static_assert(disagreements(xstd::bit_array<0>()) == 0);
        auto wrong = 0;
        for (auto const n : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 63UZ, 64UZ, 65UZ, 130UZ}) {
                wrong += disagreements(test::algorithm::patterned(xstd::bit_vector(n)));
                wrong += disagreements(test::algorithm::patterned(xstd::basic_bit_vector<std::uint8_t>(n)));
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A whole view turns the bits it views: position 0 of thirteen, turned by one, lands on 12.
BOOST_AUTO_TEST_CASE(AWholeViewRotatesWhatItViews)
{
        auto a = xstd::basic_bit_array<std::uint8_t, 13>();
        a[0]   = true;
        auto s = xstd::bit_span(a);
        xstd::bit_rotate(s, std::ranges::next(std::ranges::begin(s)));
        BOOST_CHECK(a[12] and std::ranges::count(a, true) == 1);
}

// Not a window, which shares its end blocks, nor anything const, and an rvalue owner's subrange dangles as in std::ranges.
BOOST_AUTO_TEST_CASE(WhatItRotates)
{
        using A = xstd::bit_array<8>;
        static_assert(rotatable<A&> and rotatable<xstd::bit_vector&> and rotatable<decltype(xstd::bit_span(std::declval<A&>()))>);
        static_assert(not rotatable<A const&> and not rotatable<decltype(xstd::bit_span(std::declval<A const&>()))>);
        static_assert(not rotatable<decltype(xstd::bit_span(std::declval<A&>()).subspan(1UZ, 4UZ))>);
        static_assert(not rotatable<std::vector<bool>&>);
        static_assert(std::same_as<decltype(xstd::bit_rotate(A(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
