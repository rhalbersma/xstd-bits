//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>               // bools_of, for_each_sequence, patterned, sequences
#include <test/for_each_type.hpp>                     // for_each_type
#include <xstd/bits/bit_algorithm/bit_shift_left.hpp> // bit_shift_left
#include <xstd/bits/bit_array.hpp>                    // bit_array
#include <xstd/bits/bit_fixed_set.hpp>                // bit_fixed_set
#include <xstd/bits/bit_span.hpp>                     // bit_span
#include <xstd/bits/bit_valarray.hpp>                 // bit_valarray
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                  // equal, shift_left
#include <concepts>                                   // same_as
#include <cstddef>                                    // ptrdiff_t, size_t
#include <ranges>                                     // begin, dangling, end, subrange
#include <utility>                                    // declval, forward
#include <vector>                                     // vector

BOOST_AUTO_TEST_SUITE(BitShiftLeft)

namespace {

template<class R>
concept shiftable = requires (R&& r) { xstd::bit_shift_left(std::forward<R>(r), 1); };

// The algorithm on r and std::shift_left on its bools: where the subranges lie, and what stays, before the new end.
template<class R>
[[nodiscard]] constexpr auto agrees(R&& r, std::ptrdiff_t n)
        -> bool
{
        auto model      = test::algorithm::bools_of(r);
        auto const want = std::shift_left(model.begin(), model.end(), n);
        auto const got  = xstd::bit_shift_left(r, n);
        return got.begin() == std::ranges::begin(r) and got.end() - got.begin() == want - model.begin() and std::ranges::equal(got, std::ranges::subrange(model.begin(), want));
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
BOOST_AUTO_TEST_CASE(AgreesWithRangesShiftLeftOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// In a constant expression, and over whole writable sequences of ours alone: no window, which shares its end blocks.
BOOST_AUTO_TEST_CASE(ShiftsWholeWritableSequencesAlone)
{
        static_assert([] -> bool {
                auto a = test::algorithm::patterned(xstd::bit_array<70>());
                return agrees(a, 3);
        }());
        static_assert(shiftable<xstd::bit_array<8>&> and shiftable<xstd::bit_valarray&> and shiftable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()))>);
        static_assert(not shiftable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()).subspan(1UZ, 3UZ))>);
        static_assert(not shiftable<xstd::bit_array<8> const&> and not shiftable<std::vector<bool>&> and not shiftable<xstd::bit_fixed_set<8>&>);
        static_assert(std::same_as<decltype(xstd::bit_shift_left(xstd::bit_array<8>(), 1)), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
