//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>           // in_pattern, patterned, writes_as_model, writes_through_iterators_as_model
#include <xstd/bits/bit_algorithm/bit_rotate.hpp> // bit_rotate
#include <xstd/bits/bit_array.hpp>                // basic_bit_array, bit_array
#include <xstd/bits/bit_span/bit_span.hpp>        // bit_span
#include <xstd/bits/bit_vector.hpp>               // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                              // copy, count, equal, min, rotate
#include <concepts>                               // same_as
#include <cstddef>                                // ptrdiff_t, size_t
#include <cstdint>                                // uint8_t
#include <iterator>                               // next
#include <ranges>                                 // begin, dangling, end, iota, iterator_t, size
#include <utility>                                // declval, forward
#include <vector>                                 // vector

BOOST_AUTO_TEST_SUITE(BitRotate)

namespace {

template<class R>
concept rotatable = requires (R&& r) { xstd::ranges::bit_rotate(std::forward<R>(r), std::ranges::begin(r)); };

template<class I>
concept rotatable_between = requires (I i) { xstd::ranges::bit_rotate(i, i, i); xstd::bit_rotate(i, i, i); };

// The algorithm on s against std::rotate on the same bools as ints, and the subrange it answers, at one turn.
template<class S>
[[nodiscard]] constexpr auto agrees(S s, std::size_t turn)
        -> bool
{
        auto model   = std::vector<int>(s.begin(), s.end());
        auto const t = static_cast<std::ptrdiff_t>(turn);
        std::ranges::rotate(model, model.begin() + t);
        auto const [first, last] = xstd::ranges::bit_rotate(s, std::ranges::next(std::ranges::begin(s), t));
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

// std::ranges::rotate on the bools of m as ints: libstdc++'s before version 16 is not exact over proxy references.
template<class M>
auto rotate_as_ints(M m, std::ptrdiff_t turn)
        -> void
{
        auto ints = std::vector<int>(m.begin(), m.end());
        std::ranges::rotate(ints, ints.begin() + turn);
        std::ranges::copy(ints, m.begin());
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
                wrong += disagreements(test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(n)));
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A whole view turns the bits it views: position 0 of thirteen, turned by one, lands on 12.
BOOST_AUTO_TEST_CASE(AWholeViewRotatesWhatItViews)
{
        auto a = xstd::basic_bit_array<std::uint8_t, 13>();
        a[0]   = true;
        auto s = xstd::bit_span(a);
        xstd::ranges::bit_rotate(s, std::ranges::next(std::ranges::begin(s)));
        BOOST_CHECK(a[12] and std::ranges::count(a, true) == 1);
}

// A window turns the positions it spans and leaves its end blocks' others be, at every offset, length and turn.
BOOST_AUTO_TEST_CASE(AWindowRotatesItsOwnPositions)
{
        auto wrong = 0;
        for (auto const turn : {0UZ, 1UZ, 3UZ, 8UZ, 13UZ, 64UZ, 65UZ}) {
                auto const middle = [turn](auto const& r) -> std::ptrdiff_t { return static_cast<std::ptrdiff_t>(std::ranges::min(turn, std::ranges::size(r))); };
                auto const rotate = [&](auto w) -> void { xstd::ranges::bit_rotate(w, std::ranges::begin(w) + middle(w)); };
                auto const model  = [&](auto m) -> void { rotate_as_ints(m, middle(m)); };
                wrong += test::algorithm::writes_as_model(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), rotate, model);
                wrong += test::algorithm::writes_as_model(test::algorithm::patterned(xstd::bit_array<140>()), rotate, model);
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// Between two iterators, as std::ranges::rotate(first, middle, last) and std::rotate, with the result each hands back.
BOOST_AUTO_TEST_CASE(RotatesBetweenTwoIterators)
{
        auto wrong = 0;
        for (auto const turn : {0, 1, 3, 8, 13, 64, 65}) {
                auto const middle = [turn](auto first, auto last) { return first + std::ranges::min(static_cast<std::ptrdiff_t>(turn), last - first); };
                auto const model  = [&](auto m) -> void { rotate_as_ints(m, middle(m.begin(), m.end()) - m.begin()); };
                auto const ranges = [&](auto first, auto last) -> bool {
                        auto const mid      = middle(first, last);
                        auto const [lo, hi] = xstd::ranges::bit_rotate(first, mid, last);
                        return lo == first + (last - mid) and hi == last;
                };
                auto const classic = [&](auto first, auto last) -> bool {
                        auto const mid = middle(first, last);
                        return xstd::bit_rotate(first, mid, last) == first + (last - mid);
                };
                wrong += test::algorithm::writes_through_iterators_as_model(test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(21)), ranges, model);
                wrong += test::algorithm::writes_through_iterators_as_model(test::algorithm::patterned(xstd::bit_array<140>()), classic, model);
        }
        BOOST_CHECK_EQUAL(wrong, 0);
        static_assert([] -> bool {
                auto b = test::algorithm::patterned(xstd::bit_array<70>());
                xstd::bit_rotate(b.begin() + 3, b.begin() + 4, b.end() - 2);
                return static_cast<bool>(b[3]) == test::algorithm::in_pattern(4);
        }());
}

// Anything writable of ours, windows and iterators included, but nothing const; an rvalue owner's subrange dangles.
BOOST_AUTO_TEST_CASE(WhatItRotates)
{
        using A = xstd::bit_array<8>;
        static_assert(rotatable<A&> and rotatable<xstd::bit_vector&> and rotatable<decltype(xstd::bit_span(std::declval<A&>()))>);
        static_assert(rotatable<decltype(xstd::bit_span(std::declval<A&>()).subspan(1UZ, 4UZ))>);
        static_assert(not rotatable<A const&> and not rotatable<decltype(xstd::bit_span(std::declval<A const&>()))>);
        static_assert(not rotatable<std::vector<bool>&>);
        static_assert(rotatable_between<std::ranges::iterator_t<A>> and not rotatable_between<std::ranges::iterator_t<A const>>);
        static_assert(not rotatable_between<std::vector<bool>::iterator>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_rotate(A(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
