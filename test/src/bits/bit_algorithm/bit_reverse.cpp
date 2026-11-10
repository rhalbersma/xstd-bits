//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>            // in_pattern, patterned, writes_as_model, writes_through_iterators_as_model
#include <xstd/bits/bit_algorithm/bit_reverse.hpp> // bit_reverse
#include <xstd/bits/bit_array.hpp>                 // basic_bit_array, bit_array
#include <xstd/bits/bit_span/bit_span.hpp>         // bit_span
#include <xstd/bits/bit_vector.hpp>                // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                               // count, equal, reverse
#include <concepts>                                // same_as
#include <cstdint>                                 // uint8_t
#include <ranges>                                  // dangling, end, iterator_t
#include <utility>                                 // declval, forward
#include <vector>                                  // vector

BOOST_AUTO_TEST_SUITE(BitReverse)

namespace {

template<class R>
concept reversible = requires (R&& r) { xstd::ranges::bit_reverse(std::forward<R>(r)); };

template<class I>
concept reversible_between = requires (I i) { xstd::ranges::bit_reverse(i, i); xstd::bit_reverse(i, i); };

// The algorithm on s against std::ranges::reverse on the same bools, and the end iterator it answers.
template<class S>
[[nodiscard]] constexpr auto agrees(S s)
        -> bool
{
        auto model = std::vector<bool>(s.begin(), s.end());
        std::ranges::reverse(model);
        auto const last = xstd::ranges::bit_reverse(s);
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
                wrong += static_cast<int>(not agrees(test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(n))));
        }
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A whole view reverses the bits it views, and the padding stays clear for the owner's count to see.
BOOST_AUTO_TEST_CASE(AWholeViewReversesWhatItViews)
{
        auto a = xstd::basic_bit_array<std::uint8_t, 13>();
        a[0]   = true;
        xstd::ranges::bit_reverse(xstd::bit_span(a));
        BOOST_CHECK(a[12] and std::ranges::count(a, true) == 1);
        static_assert(std::same_as<decltype(xstd::ranges::bit_reverse(xstd::bit_span(a))), std::ranges::iterator_t<decltype(xstd::bit_span(a))>>);
}

// A window reverses the positions it spans and leaves its end blocks' other positions be, at every offset and length.
BOOST_AUTO_TEST_CASE(AWindowReversesItsOwnPositions)
{
        auto const reverse = [](auto w) -> void { xstd::ranges::bit_reverse(w); };
        auto const model   = [](auto m) -> void { std::ranges::reverse(m); };
        BOOST_CHECK_EQUAL(test::algorithm::writes_as_model(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), reverse, model), 0);
        BOOST_CHECK_EQUAL(test::algorithm::writes_as_model(test::algorithm::patterned(xstd::bit_array<140>()), reverse, model), 0);
}

// Between two iterators, as std::ranges::reverse(first, last) and as std::reverse, the end handed back where asked.
BOOST_AUTO_TEST_CASE(ReversesBetweenTwoIterators)
{
        auto const model = [](auto m) -> void { std::ranges::reverse(m); };
        auto const a     = test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(21));
        BOOST_CHECK_EQUAL(test::algorithm::writes_through_iterators_as_model(a, [](auto first, auto last) -> bool { return xstd::ranges::bit_reverse(first, last) == last; }, model), 0);
        BOOST_CHECK_EQUAL(test::algorithm::writes_through_iterators_as_model(test::algorithm::patterned(xstd::bit_array<140>()), [](auto first, auto last) -> bool { xstd::bit_reverse(first, last); return true; }, model), 0);
        static_assert([] -> bool {
                auto b = test::algorithm::patterned(xstd::bit_array<70>());
                xstd::bit_reverse(b.begin() + 3, b.end() - 2);
                return static_cast<bool>(b[3]) == test::algorithm::in_pattern(67);
        }());
}

// Anything writable of ours, a window or its iterators included, but nothing const, and an rvalue owner's end dangles.
BOOST_AUTO_TEST_CASE(WhatItReverses)
{
        using A = xstd::bit_array<8>;
        static_assert(reversible<A&> and reversible<xstd::bit_vector&> and reversible<decltype(xstd::bit_span(std::declval<A&>()))>);
        static_assert(reversible<decltype(xstd::bit_span(std::declval<A&>()).subspan(1UZ, 4UZ))>);
        static_assert(not reversible<A const&> and not reversible<decltype(xstd::bit_span(std::declval<A const&>()))>);
        static_assert(not reversible<std::vector<bool>&>);
        static_assert(reversible_between<std::ranges::iterator_t<A>> and not reversible_between<std::ranges::iterator_t<A const>>);
        static_assert(not reversible_between<std::vector<bool>::iterator>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_reverse(A())), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
