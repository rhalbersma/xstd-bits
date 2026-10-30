//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                          // block_types
#include <test/for_each_type.hpp>                        // for_each_type
#include <xstd/bits/bit/bit_compress.hpp>                // bit_compress
#include <xstd/bits/bit/bit_convert.hpp>                 // bit_convert
#include <xstd/bits/bit_array.hpp>                       // basic_bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>           // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>                 // bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>            // bit_set_view
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // bit_fixed_valarray
#include <xstd/bits/bit_vector.hpp>                      // bit_vector
#include <xstd/ints/cstdint/bit_int.hpp>                 // bit_uint
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                         // array
#include <bitset>                                        // bitset
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint64_t, uint8_t
#include <ranges>                                        // iota
#include <utility>                                       // as_const

BOOST_AUTO_TEST_SUITE(BitCompress)

namespace {

// A board of 16 files and 12 ranks, square f + 16 r, wider than any one block.
inline constexpr auto files = 16UZ;
inline constexpr auto ranks = 12UZ;

using board = xstd::bit_fixed_set<files * ranks>;

// A rook's relevant occupancy on square sq: its rank and file, less sq and the board's edges, as magic tables take it.
[[nodiscard]] constexpr auto rook_mask(std::size_t sq)
        -> board
{
        auto const f = sq % files;
        auto const r = sq / files;
        auto mask    = board();
        for (auto const g : std::views::iota(1UZ, files - 1UZ)) {
                if (g != f) {
                        mask.insert(g + (r * files));
                }
        }
        for (auto const s : std::views::iota(1UZ, ranks - 1UZ)) {
                if (s != r) {
                        mask.insert(f + (s * files));
                }
        }
        return mask;
}

// The model: the k-th position of m is set in the result exactly where x holds it.
template<class Set>
[[nodiscard]] auto model_compress(Set const& x, Set const& m)
        -> Set
{
        auto result = Set();
        auto rank   = 0UZ;
        for (auto const key : m) {
                if (x.contains(key)) {
                        result.insert(rank);
                }
                ++rank;
        }
        return result;
}

// A pattern that sets a different third of the positions in every block, so blocks pack at shifting offsets.
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

// A packed set's keys as the bits of an integer index.
[[nodiscard]] auto as_index(board const& packed)
        -> std::uint64_t
{
        auto index = std::uint64_t{0};
        for (auto const key : packed) {
                index |= std::uint64_t{1} << key;
        }
        return index;
}

template<class X, class M>
concept compresses = requires (X const& x, M const& m) { xstd::bit_compress(x, m); };

} // namespace

// A rook's attack-table index on a 192-square board: three blocks' occupancy, packed into one integer.
BOOST_AUTO_TEST_CASE(ARookIndexesItsAttackTableOnALargeBoard)
{
        auto const occupied = board{3, 20, 37, 40, 69, 100, 117, 133, 150, 166, 191};
        for (auto const sq : {0UZ, 37UZ, 100UZ, 191UZ}) {
                auto const mask  = rook_mask(sq);
                auto const index = xstd::bit_compress<std::uint64_t>(occupied, mask);
                BOOST_CHECK_EQUAL(index, as_index(model_compress(occupied, mask)));
                BOOST_CHECK(xstd::bit_compress(occupied, mask) == model_compress(occupied, mask));
#if defined(__BITINT_MAXWIDTH__) && __BITINT_MAXWIDTH__ >= 192
                using word = xstd::bit_uint<192>;
                BOOST_CHECK(xstd::bit_compress(xstd::bit_convert<word>(occupied), xstd::bit_convert<word>(mask)) == word{index});
#endif
        }
}

// Every block type packs each block's selection directly above the ones below it, across every boundary.
BOOST_AUTO_TEST_CASE(EveryBlockTypePacksAcrossBlocksInOrder)
{
        test::for_each_type<test::block_types>([]<class Block> -> void {
                using set = xstd::basic_bit_fixed_set<std::size_t, Block, 257>;
                for (auto const phase : {0UZ, 1UZ, 2UZ}) {
                        auto const x = pattern<set>(257, 1, phase);
                        auto const m = pattern<set>(257, 2, 1);
                        BOOST_CHECK(xstd::bit_compress(x, m) == model_compress(x, m));
                        BOOST_CHECK(xstd::bit_compress(x, ~set()) == x);
                        BOOST_CHECK(xstd::bit_compress(x, set()) == set());
                }
        });
}

// A set of a run-time width reads keys past its width as absent, whichever of the two is wider.
BOOST_AUTO_TEST_CASE(ARunTimeSetReadsAbsentKeysPastItsWidth)
{
        BOOST_CHECK((xstd::bit_compress(xstd::bit_set{1, 500}, xstd::bit_set{1, 2, 3, 500, 900}) == xstd::bit_set{0, 3}));
        BOOST_CHECK((xstd::bit_compress(xstd::bit_set{1, 2000}, xstd::bit_set{1}) == xstd::bit_set{0}));
        BOOST_CHECK((xstd::bit_compress(xstd::bit_set{}, xstd::bit_set{4, 5}) == xstd::bit_set{}));
}

// A sequence keeps its size, the packed positions first and false after them.
BOOST_AUTO_TEST_CASE(ASequenceKeepsItsSize)
{
        auto const x = xstd::bit_vector{true, false, true, true, false};
        auto const m = xstd::bit_vector{true, true, false, true, true};
        BOOST_CHECK((xstd::bit_compress(x, m) == xstd::bit_vector{true, false, true, false, false}));

        auto const a = xstd::basic_bit_array<std::uint8_t, 20>{true, false, false, true};
        auto const b = xstd::basic_bit_array<std::uint8_t, 20>{false, true, true, true};
        BOOST_CHECK_EQUAL(xstd::bit_compress<std::uint8_t>(a, b), 0b100U);
}

// An integer that the mask's first block fills is left as it is by the empty blocks after it.
BOOST_AUTO_TEST_CASE(AnIntegerFilledByTheFirstBlockIgnoresTheRest)
{
        BOOST_CHECK_EQUAL(xstd::bit_compress<std::uint8_t>(board{1, 3, 100}, board{0, 1, 2, 3, 4, 5, 6, 7}), 0b1010U);
}

// A valarray's mask subscript gathers what bit_compress packs: the two run one kernel.
BOOST_AUTO_TEST_CASE(AValarrayGathersWhatItPacks)
{
        auto const x = xstd::bit_convert<xstd::bit_fixed_valarray<192>>(board{3, 70, 130, 191});
        auto const m = xstd::bit_convert<xstd::bit_fixed_valarray<192>>(board{3, 4, 70, 71, 130, 190, 191});
        BOOST_CHECK_EQUAL(xstd::bit_compress<std::uint64_t>(x, m), xstd::bit_convert<std::uint64_t>(std::as_const(x)[m]));
}

// The index of a fixed width is a constant where its operands are.
BOOST_AUTO_TEST_CASE(AFixedWidthCompressesInAConstantExpression)
{
        static_assert(xstd::bit_compress<std::uint64_t>(board{100, 150}, board{100, 120, 150}) == 0b101U);
        static_assert(xstd::bit_compress(board{100, 150}, board{100, 120, 150}) == board{0, 2});
        BOOST_CHECK(true);
}

// One owner of one type on both sides: the same bits read as a sequence, a view or a std::bitset are turned away.
BOOST_AUTO_TEST_CASE(OnlyTwoOwnersOfOneTypeCompress)
{
        static_assert(compresses<board, board> and compresses<xstd::bit_vector, xstd::bit_vector>);
        static_assert(not compresses<board, xstd::basic_bit_array<std::size_t, 192>>);
        static_assert(not compresses<xstd::bit_set_view<std::array<std::uint64_t, 3>>, xstd::bit_set_view<std::array<std::uint64_t, 3>>>);
        static_assert(not compresses<std::bitset<192>, std::bitset<192>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
