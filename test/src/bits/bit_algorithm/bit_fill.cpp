//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>         // bools_of, for_each_sequence, patterned, sequences, writes_as_model
#include <test/for_each_type.hpp>               // for_each_type
#include <xstd/bits/bit_algorithm/bit_fill.hpp> // bit_fill
#include <xstd/bits/bit_array.hpp>              // basic_bit_array, bit_array
#include <xstd/bits/bit_fixed_set.hpp>          // bit_fixed_set
#include <xstd/bits/bit_span.hpp>               // bit_span
#include <xstd/bits/bit_valarray.hpp>           // bit_valarray
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                            // fill
#include <concepts>                             // same_as
#include <cstdint>                              // uint8_t
#include <ranges>                               // dangling, end
#include <utility>                              // declval, forward
#include <vector>                               // vector

BOOST_AUTO_TEST_SUITE(BitFill)

namespace {

template<class R>
concept fillable = requires (R&& r) { xstd::bit_fill(std::forward<R>(r), true); };

// The algorithm on x and std::ranges::fill on its bools, through the owner or a whole view: the bools and the end.
template<class X>
[[nodiscard]] constexpr auto agrees(X x, bool value, bool through_view)
        -> bool
{
        auto model = test::algorithm::bools_of(x);
        std::ranges::fill(model, value);
        auto const ends  = [&](auto&& r) -> bool { return xstd::bit_fill(r, value) == std::ranges::end(r); };
        auto const ended = through_view ? ends(xstd::bit_span(x)) : ends(x);
        return ended and test::algorithm::bools_of(x) == model;
}

} // namespace

// Every input of every column of ours, filled either way, as an owner and through a whole view.
BOOST_AUTO_TEST_CASE(AgreesWithRangesFillOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void {
                        for (auto const value : {false, true}) {
                                wrong += static_cast<int>(not agrees(x, value, false)) + static_cast<int>(not agrees(x, value, true));
                        }
                });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window fills its own positions and leaves every other one as it was, at every offset and length.
BOOST_AUTO_TEST_CASE(AWindowFillsItsOwnPositionsAlone)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        for (auto const value : {false, true}) {
                BOOST_CHECK_EQUAL(test::algorithm::writes_as_model(a, [=](auto w) -> void { static_cast<void>(xstd::bit_fill(w, value)); }, [=](auto m) -> void { std::ranges::fill(m, value); }), 0);
        }
}

// In a constant expression, and over writable bools of ours alone: never a const owner, a set or std::vector<bool>.
BOOST_AUTO_TEST_CASE(FillsOurWritableSequencesAlone)
{
        static_assert(agrees(test::algorithm::patterned(xstd::bit_array<70>()), true, false) and agrees(xstd::bit_array<70>(), true, true));
        static_assert(fillable<xstd::bit_array<8>&> and fillable<xstd::bit_valarray&> and fillable<decltype(xstd::bit_span(std::declval<xstd::bit_array<8>&>()).subspan(1UZ, 3UZ))>);
        static_assert(not fillable<xstd::bit_array<8> const&> and not fillable<std::vector<bool>&> and not fillable<xstd::bit_fixed_set<8>&>);
        static_assert(std::same_as<decltype(xstd::bit_fill(xstd::bit_array<8>(), true)), std::ranges::dangling>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
