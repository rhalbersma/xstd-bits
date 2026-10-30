//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                // block_types
#include <test/for_each_type.hpp>              // for_each_type
#include <xstd/bits/bit/bit_compress.hpp>      // bit_compress
#include <xstd/bits/bit/bit_expand.hpp>        // bit_expand
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>       // bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>  // bit_set_view
#include <xstd/bits/bit_vector.hpp>            // bit_vector
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                               // array
#include <bitset>                              // bitset
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t, uint8_t
#include <ranges>                              // iota
#include <set>                                 // set

BOOST_AUTO_TEST_SUITE(BitExpand)

namespace {

using board = xstd::bit_fixed_set<192>;

// Positions in a third of every block, at a phase each block shifts, so the deposits cross every boundary.
template<class Set>
[[nodiscard]] auto pattern(std::size_t n, std::size_t stride, std::size_t phase)
        -> Set
{
        auto result = Set();
        for (auto const i : std::views::iota(0UZ, n)) {
                if (((i * stride) + phase) % 3UZ == 0UZ) {
                        result.insert(i);
                }
        }
        return result;
}

template<class X, class M>
concept expands = requires (X const& x, M const& m) { xstd::bit_expand(x, m); };

} // namespace

// Expanding what was compressed through the same mask gives back the source's positions that the mask holds.
BOOST_AUTO_TEST_CASE(ExpandUndoesCompressThroughTheSameMask)
{
        test::for_each_type<test::block_types>([]<class Block> -> void {
                using set = xstd::basic_bit_fixed_set<std::size_t, Block, 257>;
                for (auto const phase : {0UZ, 1UZ, 2UZ}) {
                        auto const x = pattern<set>(257, 1, phase);
                        auto const m = pattern<set>(257, 2, 1);
                        BOOST_CHECK(xstd::bit_expand(xstd::bit_compress(x, m), m) == (x & m));
                        BOOST_CHECK(xstd::bit_compress(xstd::bit_expand(x, m), m) == (x & ~set() >> (257 - m.size())));
                }
        });
}

// An index walks the subsets of a mask that spans three blocks, each exactly once, as a magic table is filled.
BOOST_AUTO_TEST_CASE(AnIndexWalksEverySubsetOfAMask)
{
        auto const mask = board{5, 63, 64, 100, 127, 128, 191};
        auto seen       = std::set<std::uint64_t>();
        for (auto const index : std::views::iota(std::uint64_t{0}, std::uint64_t{1} << mask.size())) {
                auto const subset = xstd::bit_expand(index, mask);
                BOOST_CHECK((subset & mask) == subset);
                BOOST_CHECK_EQUAL(xstd::bit_compress<std::uint64_t>(subset, mask), index);
                seen.insert(xstd::bit_compress<std::uint64_t>(subset, mask));
        }
        BOOST_CHECK_EQUAL(seen.size(), std::size_t{1} << mask.size());
}

// A run-time set narrower than the mask reads its absent keys as false, a wider one lays out only its low keys.
BOOST_AUTO_TEST_CASE(ARunTimeWidthLaysItsLowBitsOut)
{
        BOOST_CHECK((xstd::bit_expand(xstd::bit_set{0, 3}, xstd::bit_set{1, 2, 3, 500, 900}) == xstd::bit_set{1, 500}));
        BOOST_CHECK((xstd::bit_expand(xstd::bit_set{1}, xstd::bit_set{7, 900}) == xstd::bit_set{900}));
        BOOST_CHECK((xstd::bit_expand(xstd::bit_set{0, 2, 2000}, xstd::bit_set{4, 5, 6}) == xstd::bit_set{4, 6}));
        auto const x = xstd::bit_vector{true, false, true, false, false};
        auto const m = xstd::bit_vector{false, true, true, false, true};
        BOOST_CHECK((xstd::bit_expand(x, m) == xstd::bit_vector{false, true, false, false, true}));
}

// An integer wider than every block of a small mask together lays out the mask's positions and nothing past its width.
BOOST_AUTO_TEST_CASE(AnIntegerWiderThanTheMaskLaysOutWhatFits)
{
        using small = xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 20>;
        BOOST_CHECK((xstd::bit_expand(~std::uint64_t{0}, small{1, 9, 19}) == small{1, 9, 19}));
}

// The deposit of a fixed width is a constant where its operands are.
BOOST_AUTO_TEST_CASE(AFixedWidthExpandsInAConstantExpression)
{
        static_assert(xstd::bit_expand(std::uint64_t{0b101}, board{100, 120, 150}) == board{100, 150});
        static_assert(xstd::bit_expand(board{0, 2}, board{100, 120, 150}) == board{100, 150});
        BOOST_CHECK(true);
}

// One owner of one type on both sides, or an integer and an owner: views and std::bitset are turned away.
BOOST_AUTO_TEST_CASE(OnlyTwoOwnersOfOneTypeExpand)
{
        static_assert(expands<board, board> and expands<std::uint64_t, board>);
        static_assert(not expands<xstd::bit_set_view<std::array<std::uint64_t, 3>>, xstd::bit_set_view<std::array<std::uint64_t, 3>>>);
        static_assert(not expands<std::bitset<192>, std::bitset<192>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
