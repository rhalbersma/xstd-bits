//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/algorithm/sequences.hpp>                // bools_of, for_each_sequence, for_each_window, offsets, patterned, sequences, striped_vector
#include <test/for_each_type.hpp>                      // for_each_type
#include <xstd/bits/bit_algorithm/bit_rotate_copy.hpp> // bit_rotate_copy
#include <xstd/bits/bit_array.hpp>                     // basic_bit_array, bit_array
#include <xstd/bits/bit_span/bit_span.hpp>             // bit_span
#include <xstd/bits/bit_vector.hpp>                    // bit_vector
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                   // rotate_copy, rotate_copy_result
#include <concepts>                                    // same_as
#include <cstddef>                                     // ptrdiff_t, size_t
#include <cstdint>                                     // uint64_t, uint8_t
#include <ranges>                                      // begin, dangling, end, iterator_t, size
#include <utility>                                     // declval
#include <vector>                                      // vector

BOOST_AUTO_TEST_SUITE(BitRotateCopy)

namespace {

template<class R, class O>
concept copyable = requires (R const& r, O out) { xstd::ranges::bit_rotate_copy(r, std::ranges::begin(r), out); };

// The algorithm and std::ranges::rotate_copy from r at a turn into a striped destination over Block at an offset.
template<class Block, class R>
[[nodiscard]] auto agrees(R const& r, std::size_t turn, std::size_t offset)
        -> bool
{
        auto const source = test::algorithm::bools_of(r);
        auto dst          = test::algorithm::striped_vector<Block>(source.size() + offset + 5UZ);
        auto model        = test::algorithm::bools_of(dst);
        auto const t      = static_cast<std::ptrdiff_t>(turn);
        auto const d      = static_cast<std::ptrdiff_t>(offset);
        auto const want   = std::ranges::rotate_copy(source, source.begin() + t, model.begin() + d);
        auto const got    = xstd::ranges::bit_rotate_copy(r, std::ranges::begin(r) + t, dst.begin() + d);
        return got.in == std::ranges::end(r) and got.out - dst.begin() == want.out - model.begin() and test::algorithm::bools_of(dst) == model;
}

// At the ends and inside, either side of a narrow block, into either block width at every offset.
template<class R>
[[nodiscard]] auto disagreements(R const& r)
        -> int
{
        auto const n = std::ranges::size(r);
        auto wrong   = 0;
        for (auto const turn : {0UZ, 1UZ, 7UZ, 9UZ, n / 2UZ, n}) {
                for (auto const offset : test::algorithm::offsets) {
                        if (turn <= n) {
                                wrong += static_cast<int>(not agrees<std::uint8_t>(r, turn, offset)) + static_cast<int>(not agrees<std::uint64_t>(r, turn, offset));
                        }
                }
        }
        return wrong;
}

} // namespace

// Every input of every column of ours, into two block widths at aligned and unaligned offsets.
BOOST_AUTO_TEST_CASE(AgreesWithRangesRotateCopyOverEveryColumn)
{
        test::for_each_type<test::algorithm::sequences>([]<class X> -> void {
                auto wrong = 0;
                test::algorithm::for_each_sequence<X>([&](X const& x) -> void { wrong += disagreements(x) + disagreements(xstd::bit_span(x)); });
                BOOST_CHECK_EQUAL(wrong, 0);
        });
}

// A window turns its own positions alone, at every offset and length.
BOOST_AUTO_TEST_CASE(AWindowTurnsItsOwnPositions)
{
        auto const a = test::algorithm::patterned(xstd::basic_bit_array<std::uint8_t, 21>());
        auto wrong   = 0;
        test::algorithm::for_each_window(xstd::bit_span(a), [&](auto const& w) -> void { wrong += disagreements(w); });
        BOOST_CHECK_EQUAL(wrong, 0);
}

// In a constant expression, between bools of ours alone, into a destination that writes.
BOOST_AUTO_TEST_CASE(TurnsBetweenOurSequencesAlone)
{
        static_assert([] -> bool {
                auto const a = test::algorithm::patterned(xstd::bit_array<70>());
                auto b       = xstd::bit_array<70>();
                auto const r = xstd::ranges::bit_rotate_copy(a, a.begin() + 3, b.begin());
                auto model   = test::algorithm::bools_of(a);
                auto turned  = test::algorithm::bools_of(b);
                std::ranges::rotate_copy(model, model.begin() + 3, turned.begin());
                return r.out == b.end() and test::algorithm::bools_of(b) == turned;
        }());
        using A = xstd::bit_array<8>;
        static_assert(copyable<A, std::ranges::iterator_t<xstd::bit_vector>> and not copyable<A, std::ranges::iterator_t<A const>> and not copyable<A, std::vector<bool>::iterator>);
        static_assert(not copyable<std::vector<bool>, std::ranges::iterator_t<A>>);
        static_assert(std::same_as<decltype(xstd::ranges::bit_rotate_copy(A(), std::declval<std::ranges::iterator_t<A>>(), std::declval<std::ranges::iterator_t<A>>())), std::ranges::rotate_copy_result<std::ranges::dangling, std::ranges::iterator_t<A>>>);
        BOOST_CHECK(true); // silence Boost.Test's "test case did not check any assertions"
}

BOOST_AUTO_TEST_SUITE_END()
