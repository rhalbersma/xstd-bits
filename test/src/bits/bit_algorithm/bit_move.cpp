//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>         // bools_of, for_each_sequence, for_each_window, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>               // for_each_type
#include <xstd/bits/bit_algorithm/bit_move.hpp> // bit_move
#include <xstd/bits/bit_array.hpp>              // basic_bit_array, bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>  // bit_fixed_set
#include <xstd/bits/bit_span.hpp>               // bit_span
#include <xstd/bits/bit_vector.hpp>             // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                            // move, move_result
#include <concepts>                             // same_as
#include <cstddef>                              // ptrdiff_t, size_t
#include <cstdint>                              // uint64_t, uint8_t
#include <iterator>                             // back_insert_iterator
#include <ranges>                               // dangling, end, iterator_t
#include <utility>                              // declval, pair
#include <vector>                               // vector

BOOST_AUTO_TEST_SUITE(BitMove)

namespace {

template<class R, class O>
concept movable = requires (R const& r, O out) { xstd::bit_move(r, out); };

// The algorithm and std::ranges::move from r into a striped destination over Block at an offset: bools and ends.
template<class Block, class R>
[[nodiscard]] auto agrees(R const& r, std::size_t offset)
        -> bool
{
        auto const source = test::algorithm::bools_of(r);
        auto dst          = test::algorithm::striped_vector<Block>(source.size() + offset + 5UZ);
        auto model        = test::algorithm::bools_of(dst);
        auto const d      = static_cast<std::ptrdiff_t>(offset);
        auto const want   = std::ranges::move(source, model.begin() + d);
        auto const got    = xstd::bit_move(r, dst.begin() + d);
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

// Within one sequence, a window copied to every position at or below its own, as std::ranges::move allows.
template<class Block>
[[nodiscard]] auto overlapping_disagreements()
        -> int
{
        auto const v = test::algorithm::patterned(xstd::basic_bit_vector<Block>(150UZ));
        auto wrong   = 0;
        for (auto const& [first, count] : {std::pair{9UZ, 70UZ}, std::pair{64UZ, 70UZ}, std::pair{70UZ, 3UZ}}) {
                for (auto const to : {0UZ, 1UZ, 8UZ, first - 1UZ, first}) {
                        auto w     = v;
                        auto model = test::algorithm::bools_of(v);
                        std::ranges::move(model.begin() + static_cast<std::ptrdiff_t>(first), model.begin() + static_cast<std::ptrdiff_t>(first + count), model.begin() + static_cast<std::ptrdiff_t>(to));
                        static_cast<void>(xstd::bit_move(xstd::bit_span(w).subspan(first, count), w.begin() + static_cast<std::ptrdiff_t>(to)));
                        wrong += static_cast<int>(test::algorithm::bools_of(w) != model);
                }
        }
        return wrong;
}

} // namespace

// Every input of every column of ours and the whole views over them, into two block widths at several offsets.
BOOST_AUTO_TEST_CASE(AgreesWithRangesMoveOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window copies its own positions alone, at every offset and length, to every destination offset.
BOOST_AUTO_TEST_CASE(AWindowMovesItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// A source and destination in one sequence, the destination below, overlapping or not.
BOOST_AUTO_TEST_CASE(MovesDownOverItself)
{
        BOOST_CHECK_EQUAL(overlapping_disagreements<std::uint8_t>(), 0);
        BOOST_CHECK_EQUAL(overlapping_disagreements<std::uint64_t>(), 0);
}

// In a constant expression, between bools of ours alone, into a destination that writes.
BOOST_AUTO_TEST_CASE(MovesBetweenOurSequencesAlone)
{
        static_assert([] -> bool {
                auto const a = test::algorithm::patterned(xstd::bit_array<70>());
                auto b       = xstd::bit_array<75>();
                auto const r = xstd::bit_move(a, b.begin() + 5);
                return r.out == b.end() and test::algorithm::bools_of(xstd::bit_span(b).subspan(5UZ)) == test::algorithm::bools_of(a);
        }());
        using A = xstd::bit_array<8>;
        static_assert(movable<A, std::ranges::iterator_t<xstd::bit_vector>> and movable<A, std::ranges::iterator_t<A>>);
        static_assert(not movable<A, std::ranges::iterator_t<A const>> and not movable<A, std::vector<bool>::iterator> and not movable<A, std::back_insert_iterator<std::vector<bool>>>);
        static_assert(not movable<std::vector<bool>, std::ranges::iterator_t<A>> and not movable<xstd::bit_fixed_set<8>, std::ranges::iterator_t<A>>);
        static_assert(std::same_as<decltype(xstd::bit_move(A(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::move_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
