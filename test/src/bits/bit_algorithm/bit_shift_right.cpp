//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                // bools_of, for_each_sequence, patterned, sequences, writes_as_model, writes_through_iterators_as_model
#include <test/for_each_type.hpp>                      // for_each_type
#include <xstd/bits/bit_algorithm/bit_shift_right.hpp> // bit_shift_right
#include <xstd/bits/bit_array.hpp>                     // bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>         // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>             // bit_span
#include <xstd/bits/bit_valarray/bit_valarray.hpp>     // bit_valarray
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                   // equal, shift_right
#include <concepts>                                    // same_as
#include <cstddef>                                     // ptrdiff_t, size_t
#include <ranges>                                      // begin, dangling, end, iota, iterator_t, size, subrange
#include <utility>                                     // declval, forward
#include <vector>                                      // vector

BOOST_AUTO_TEST_SUITE(BitShiftRight)

namespace {

template<class R>
concept shiftable = requires (R&& r) { xstd::ranges::bit_shift_right(std::forward<R>(r), 1); };

template<class I>
concept shiftable_between = requires (I i) { xstd::ranges::bit_shift_right(i, i, 1); xstd::bit_shift_right(i, i, 1); };

// The algorithm on r and std::shift_right on its bools: where the subranges lie, and what stays, from the new begin.
template<class R>
[[nodiscard]] constexpr auto agrees(R&& r, std::ptrdiff_t n)
        -> bool
{
        auto model      = test::algorithm::bools_of(r);
        auto const want = std::shift_right(model.begin(), model.end(), n);
        auto const got  = xstd::ranges::bit_shift_right(r, n);
        return got.end() == std::ranges::end(r) and got.begin() - std::ranges::begin(r) == want - model.begin() and std::ranges::equal(got, std::ranges::subrange(want, model.end()));
}

// Every shift of x worth asking: none, by one, either side of a block, and to and past its end, as owner and as view.
template<class X>
[[nodiscard]] auto disagreements(X const& x)
        -> int
{
        auto const size = static_cast<std::ptrdiff_t>(x.size());
        auto wrong      = 0;
        for (auto const n : {std::ptrdiff_t{0}, std::ptrdiff_t{1}, std::ptrdiff_t{7}, std::ptrdiff_t{8}, std::ptrdiff_t{9}, std::ptrdiff_t{63}, std::ptrdiff_t{64}, std::ptrdiff_t{65}, size - 1, size, size + 5}) {
                if (0 <= n) {
                        auto y = x;
                        auto z = x;
                        wrong += static_cast<int>(not agrees(y, n)) + static_cast<int>(not agrees(xstd::bit_span(z), n));
                }
        }
        return wrong;
}

} // namespace

// Every input of every column of ours, as an owner and through a whole view, across blocks, widths and storages.
BOOST_AUTO_TEST_CASE(AgreesWithRangesShiftRightOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window shifts the positions it spans, false into those vacated, and leaves its end blocks' other positions be.
BOOST_AUTO_TEST_CASE(AWindowShiftsItsOwnPositions)
{
        auto wrong = 0;
        for (auto const n : {0UZ, 1UZ, 3UZ, 8UZ, 13UZ, 64UZ, 65UZ, 200UZ}) {
        auto const model = [n](auto m) -> void {
                auto const at = [&m](std::size_t i) -> auto { return m.begin()[static_cast<std::ptrdiff_t>(i)]; };
                auto const z  = std::ranges::size(m);
                if (n == 0UZ or z <= n) {
                        return;
                }
                for (auto const k : std::views::iota(0UZ, z - n)) {
                        at(z - 1UZ - k) = static_cast<bool>(at(z - 1UZ - k - n));
                }
                for (auto const i : std::views::iota(0UZ, n)) {
                        at(i) = false;
                }
        };
                auto const shift = [n](auto w) -> void { xstd::ranges::bit_shift_right(w, static_cast<std::ptrdiff_t>(n)); };
                wrong += test::algorithm::writes_as_model(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), shift, model);
                wrong += test::algorithm::writes_as_model(test::algorithm::patterned(xstd::bit_array<140>()), shift, model);
                auto const d       = static_cast<std::ptrdiff_t>(n);
                auto const ranges  = [d](auto first, auto last) -> bool { auto const [lo, hi] = xstd::ranges::bit_shift_right(first, last, d); return lo == (d < last - first ? first + d : last) and hi == last; };
                auto const classic = [d](auto first, auto last) -> bool { return xstd::bit_shift_right(first, last, d) == (d < last - first ? first + d : last); };
                wrong += test::algorithm::writes_through_iterators_as_model(test::algorithm::patterned(xstd::basic_bit_vector<std::vector<std::uint8_t>>(21)), ranges, model);
                wrong += test::algorithm::writes_through_iterators_as_model(test::algorithm::patterned(xstd::bit_array<140>()), classic, model);
        }
        BOOST_CHECK_EQUAL(wrong, 0);
        static_assert([] -> bool {
                auto b = test::algorithm::patterned(xstd::bit_array<70>());
                xstd::bit_shift_right(b.begin() + 3, b.end() - 2, 1);
                return static_cast<bool>(b[4]) == test::algorithm::in_pattern(3);
        }());
}

// In a constant expression, and over writable sequences of ours alone, a window and a pair of iterators included.
BOOST_AUTO_TEST_CASE(ShiftsWritableSequencesAlone)
{
        static_assert([] -> bool {
                auto a = test::algorithm::patterned(xstd::bit_array<70>());
                return agrees(a, 3);
        }());
        static_assert(shiftable<xstd::bit_array<8>&> and shiftable<xstd::bit_valarray&> and shiftable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()))>);
        static_assert(shiftable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()).subspan(1UZ, 3UZ))>);
        static_assert(shiftable_between<std::ranges::iterator_t<xstd::bit_array<8>>> and not shiftable_between<std::ranges::iterator_t<xstd::bit_array<8> const>>);
        static_assert(not shiftable_between<std::vector<bool>::iterator>);
        static_assert(not shiftable<xstd::bit_array<8> const&> and not shiftable<std::vector<bool>&> and not shiftable<xstd::bit_fixed_set<8>&>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_shift_right(xstd::bit_array<8>(), 1)), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
