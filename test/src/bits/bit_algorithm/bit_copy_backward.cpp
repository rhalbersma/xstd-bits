//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                  // agrees_through_iterators, bools_of, for_each_sequence, for_each_window, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>                        // for_each_type
#include <xstd/bits/bit_algorithm/bit_copy_backward.hpp> // bit_copy_backward
#include <xstd/bits/bit_array.hpp>                       // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>           // bit_fixed_set
#include <xstd/bits/bit_span/bit_span.hpp>               // bit_span
#include <xstd/bits/bit_vector.hpp>                      // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                     // copy_backward, copy_backward_result
#include <concepts>                                      // same_as
#include <cstddef>                                       // ptrdiff_t, size_t
#include <cstdint>                                       // uint64_t, uint8_t
#include <ranges>                                        // dangling, end, iterator_t, size
#include <utility>                                       // declval, pair
#include <vector>                                        // vector

BOOST_AUTO_TEST_SUITE(BitCopyBackward)

namespace {

template<class I>
concept copy_backward_between = requires (I i) { xstd::ranges::bit_copy_backward(i, i, xstd::bit_vector().end()); xstd::bit_copy_backward(i, i, xstd::bit_vector().end()); };

template<class R, class O>
concept copyable = requires (R const& r, O out) { xstd::ranges::bit_copy_backward(r, out); };

// The algorithm and std::ranges::copy_backward from r to end at an offset into a striped destination over Block.
template<class Block, class R>
[[nodiscard]] auto agrees(R const& r, std::size_t offset)
        -> bool
{
        auto const source = test::algorithm::bools_of(r);
        auto dst          = test::algorithm::striped_vector<Block>(source.size() + offset + 5UZ);
        auto model        = test::algorithm::bools_of(dst);
        auto const d      = static_cast<std::ptrdiff_t>(source.size() + offset);
        auto const want   = std::ranges::copy_backward(source, model.begin() + d);
        auto const got    = xstd::ranges::bit_copy_backward(r, dst.begin() + d);
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

// Within one sequence, a window copied to end at every position at or above its own end, as copy_backward allows.
template<class Block>
[[nodiscard]] auto overlapping_disagreements()
        -> int
{
        auto const v = test::algorithm::patterned(xstd::basic_bit_vector<std::vector<Block>>(150UZ));
        auto wrong   = 0;
        for (auto const& [first, count] : {std::pair{9UZ, 70UZ}, std::pair{0UZ, 70UZ}, std::pair{70UZ, 3UZ}}) {
                for (auto const to : {first + count, first + count + 1UZ, first + count + 8UZ, 141UZ, 150UZ}) {
                        auto w     = v;
                        auto model = test::algorithm::bools_of(v);
                        std::ranges::copy_backward(model.begin() + static_cast<std::ptrdiff_t>(first), model.begin() + static_cast<std::ptrdiff_t>(first + count), model.begin() + static_cast<std::ptrdiff_t>(to));
                        static_cast<void>(xstd::ranges::bit_copy_backward(xstd::bit_span(w).subspan(first, count), w.begin() + static_cast<std::ptrdiff_t>(to)));
                        wrong += static_cast<int>(test::algorithm::bools_of(w) != model);
                }
        }
        return wrong;
}

} // namespace

// Every input of every column of ours and the whole views over them, into two block widths at several offsets.
BOOST_AUTO_TEST_CASE(AgreesWithRangesCopyBackwardOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window copies its own positions alone, at every offset and length, to every destination offset.
BOOST_AUTO_TEST_CASE(AWindowCopiesItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A source and destination in one sequence, the destination above, overlapping or not.
BOOST_AUTO_TEST_CASE(CopiesUpOverItself)
{
        BOOST_CHECK_EQUAL(overlapping_disagreements<std::uint8_t>(), 0);
        BOOST_CHECK_EQUAL(overlapping_disagreements<std::uint64_t>(), 0);
}

// In a constant expression, between bools of ours alone, into a destination that writes.
BOOST_AUTO_TEST_CASE(CopiesBetweenOurSequencesAlone)
{
        static_assert([] -> bool {
                auto const a = test::algorithm::patterned(xstd::bit_array<70>());
                auto b       = xstd::bit_array<75>();
                auto const r = xstd::ranges::bit_copy_backward(a, b.end());
                return r.out == b.begin() + 5 and test::algorithm::bools_of(xstd::bit_span(b).subspan(5UZ)) == test::algorithm::bools_of(a);
        }());
        using A = xstd::bit_array<8>;
        static_assert(copyable<A, std::ranges::iterator_t<xstd::bit_vector>> and not copyable<A, std::ranges::iterator_t<A const>> and not copyable<A, std::vector<bool>::iterator>);
        static_assert(not copyable<std::vector<bool>, std::ranges::iterator_t<A>> and not copyable<xstd::bit_fixed_set<8>, std::ranges::iterator_t<A>>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_copy_backward(A(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::copy_backward_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

// From two iterators, as std::ranges::copy_backward and std::copy_backward, into a destination offset across a block.
BOOST_AUTO_TEST_CASE(CopiesBackwardFromBetweenTwoIterators)
{
        auto const agrees = [](auto first, auto last, auto const& w) -> bool {
                auto const n            = static_cast<std::ptrdiff_t>(std::ranges::size(w));
                auto want               = xstd::bit_vector(140UZ);
                auto got                = want;
                auto classic            = want;
                auto const range_result = xstd::ranges::bit_copy_backward(w, want.begin() + 90);
                auto const iter_result  = xstd::ranges::bit_copy_backward(first, last, got.begin() + 90);
                auto const classic_end  = xstd::bit_copy_backward(first, last, classic.begin() + 90);
                return got == want and classic == want and iter_result.in == last and iter_result.out - got.begin() == range_result.out - want.begin() and classic_end - classic.begin() == 90 - n;
        };
        BOOST_CHECK_EQUAL(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>()), agrees), 0);
        static_assert(test::algorithm::agrees_through_iterators(test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 9>()), agrees) == 0);
        static_assert(copy_backward_between<std::ranges::iterator_t<xstd::bit_array<8> const>> and not copy_backward_between<std::vector<bool>::const_iterator>);
}

BOOST_AUTO_TEST_SUITE_END()
