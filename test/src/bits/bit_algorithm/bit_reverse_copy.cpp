//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm/bit_reverse_copy.hpp> // bit_reverse_copy
#include <xstd/bits/bit_array.hpp>                      // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>          // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>              // bit_span
#include <xstd/bits/bit_vector.hpp>                     // bit_vector
#include <boost/test/unit_test.hpp>                     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <test/algorithm/sequences.hpp>                 // agrees_through_iterators, bools_of, for_each_sequence, for_each_window, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>                       // for_each_type
#include <algorithm>                                    // reverse, reverse_copy, reverse_copy_result
#include <concepts>                                     // same_as
#include <cstddef>                                      // ptrdiff_t, size_t
#include <cstdint>                                      // uint64_t, uint8_t
#include <iterator>                                     // back_insert_iterator
#include <ranges>                                       // dangling, end, iterator_t, size
#include <utility>                                      // declval
#include <vector>                                       // vector

BOOST_AUTO_TEST_SUITE(BitReverseCopy)

namespace {

template<class I>
concept reverse_copy_between = requires (I i) { xstd::ranges::bit_reverse_copy(i, i, xstd::bit_vector().begin()); xstd::bit_reverse_copy(i, i, xstd::bit_vector().begin()); };

template<class R, class O>
concept copyable = requires (R const& r, O out) { xstd::ranges::bit_reverse_copy(r, out); };

// The algorithm and std::ranges::reverse_copy from r into a striped destination over Block at an offset.
template<class Block, class R>
[[nodiscard]] auto agrees(R const& r, std::size_t offset)
        -> bool
{
        auto const source = test::algorithm::bools_of(r);
        auto dst          = test::algorithm::striped_vector<Block>(source.size() + offset + 5UZ);
        auto model        = test::algorithm::bools_of(dst);
        auto const d      = static_cast<std::ptrdiff_t>(offset);
        auto const want   = std::ranges::reverse_copy(source, model.begin() + d);
        auto const got    = xstd::ranges::bit_reverse_copy(r, dst.begin() + d);
        return got.in == std::ranges::end(r) and got.out - dst.begin() == want.out - model.begin() and test::algorithm::bools_of(dst) == model;
}

// Into either block width at every offset.
template<class R>
[[nodiscard]] auto disagreements(R const& r)
        -> int
{
        auto wrong = 0;
        for (auto const offset : test::algorithm::offsets) {
                wrong += static_cast<int>(not agrees<std::uint8_t>(r, offset)) + static_cast<int>(not agrees<std::uint64_t>(r, offset));
        }
        return wrong;
}

} // namespace

// Every input of every column of ours and the whole views over them, into two block widths at several offsets.
BOOST_AUTO_TEST_CASE(AgreesWithRangesReverseCopyOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window copies its own positions alone, at every offset and length, to every destination offset.
BOOST_AUTO_TEST_CASE(AWindowReversesItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, between bools of ours alone, into a destination that writes.
BOOST_AUTO_TEST_CASE(ReversesBetweenOurSequencesAlone)
{
        static_assert([] -> bool {
                auto const a  = test::algorithm::patterned(xstd::bit_array<70>());
                auto b        = xstd::bit_array<75>();
                auto const r  = xstd::ranges::bit_reverse_copy(a, b.begin() + 5);
                auto reversed = test::algorithm::bools_of(a);
                std::ranges::reverse(reversed);
                return r.out == b.end() and test::algorithm::bools_of(xstd::bit_span(b).subspan(5UZ)) == reversed;
        }());
        using A = xstd::bit_array<8>;
        static_assert(copyable<A, std::ranges::iterator_t<xstd::bit_vector>> and copyable<A, std::ranges::iterator_t<A>>);
        static_assert(not copyable<A, std::ranges::iterator_t<A const>> and not copyable<A, std::vector<bool>::iterator> and not copyable<A, std::back_insert_iterator<std::vector<bool>>>);
        static_assert(not copyable<std::vector<bool>, std::ranges::iterator_t<A>> and not copyable<xstd::bit_fixed_set<8>, std::ranges::iterator_t<A>>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_reverse_copy(A(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::reverse_copy_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// From between two iterators, as std::ranges::reverse_copy and as std::reverse_copy, into a destination at an offset across a block.
BOOST_AUTO_TEST_CASE(ReverseCopiesFromBetweenTwoIterators)
{
        auto const agrees = [](auto first, auto last, auto const& w) -> bool {
                auto const n            = static_cast<std::ptrdiff_t>(std::ranges::size(w));
                auto want               = xstd::bit_vector(140UZ);
                auto got                = want;
                auto classic            = want;
                auto const range_result = xstd::ranges::bit_reverse_copy(w, want.begin() + 67);
                auto const iter_result  = xstd::ranges::bit_reverse_copy(first, last, got.begin() + 67);
                auto const classic_end  = xstd::bit_reverse_copy(first, last, classic.begin() + 67);
                return got == want and classic == want and iter_result.in == last and iter_result.out - got.begin() == range_result.out - want.begin() and classic_end - classic.begin() == 67 + n;
        };
        BOOST_CHECK_EQUAL(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), agrees), 0);
        static_assert(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 9>()), agrees) == 0);
        static_assert(reverse_copy_between<std::ranges::iterator_t<xstd::bit_array<8> const>> and not reverse_copy_between<std::vector<bool>::const_iterator>);
}

BOOST_AUTO_TEST_SUITE_END()
