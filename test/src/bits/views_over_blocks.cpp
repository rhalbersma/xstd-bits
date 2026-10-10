//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <xstd/bits/bit_span.hpp>     // bit_span
#include <xstd/bits/bit_subspan.hpp>  // bit_subspan
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                  // ranges::equal, ranges::find, ranges::reverse
#include <array>                      // array
#include <concepts>                   // same_as
#include <cstddef>                    // size_t
#include <cstdint>                    // uint8_t, uint16_t, uint32_t, uint64_t
#include <initializer_list>           // initializer_list
#include <limits>                     // numeric_limits
#include <ranges>                     // borrowed_range, iota, range_value_t, view
#include <span>                       // span
#include <utility>                    // as_const, forward
#include <vector>                     // vector

BOOST_AUTO_TEST_SUITE(ViewsOverBlocks)

namespace {

// The positions the blocks hold, read one bit at a time: bit n of block i is position i * digits + n.
template<class R>
[[nodiscard]] auto positions_of(R const& blocks)
        -> std::vector<std::size_t>
{
        using block_type      = std::ranges::range_value_t<R>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
        auto positions        = std::vector<std::size_t>();
        auto i                = 0UZ;
        for (auto const block : blocks) {
                for (auto const n : std::views::iota(0UZ, digits)) {
                        if (((static_cast<std::uint64_t>(block) >> n) & 1U) != 0U) {
                                positions.push_back((i * digits) + n);
                        }
                }
                ++i;
        }
        return positions;
}

// Both readings of the same blocks agree with the blocks themselves, forwards and backwards.
template<class W, class R>
[[nodiscard]] auto readings_agree_with(W& viewed, R const& blocks)
        -> bool
{
        auto const expected = positions_of(blocks);
        auto const set      = xstd::bit_set_view(viewed);
        auto const seq      = xstd::bit_span(viewed);

        auto const forward = std::vector<std::size_t>(set.begin(), set.end());
        auto backward      = std::vector<std::size_t>(set.rbegin(), set.rend());
        std::ranges::reverse(backward);

        auto from_sequence = std::vector<std::size_t>();
        for (auto const n : std::views::iota(0UZ, seq.size())) {
                if (seq[n]) {
                        from_sequence.push_back(n);
                }
        }
        return forward == expected and backward == expected and from_sequence == expected and set.size() == expected.size() and seq.count() == expected.size();
}

template<class W>
concept set_viewable = requires (W&& w) { xstd::bit_set_view(std::forward<W>(w)); };

template<class W>
concept span_viewable = requires (W&& w) { xstd::bit_span(std::forward<W>(w)); };

template<class V>
concept can_insert = requires (V const& v) { v.insert(0UZ); };

template<class S>
concept can_assign_element = requires (S const& s) { s[0] = true; };

// A built-in array of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks = std::uint64_t[4]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// One block is its own digits: a static width of one block, held as a pointer and nothing else.
BOOST_AUTO_TEST_CASE(OneBlockIsItsOwnDigits)
{
        auto board         = std::uint64_t{0b1010'0101};
        auto const squares = xstd::bit_set_view(board);
        static_assert(sizeof(squares) == sizeof(std::uint64_t*));
        static_assert(sizeof(xstd::bit_span(board)) == sizeof(std::uint64_t*));
        static_assert(decltype(xstd::bit_span(board))::static_extent == 64UZ);
        BOOST_CHECK(readings_agree_with(board, std::array{board}));

        // The views write through to the block, which is what viewing it in place is for.
        BOOST_CHECK(squares.insert(63UZ).second);
        BOOST_CHECK_EQUAL(squares.erase(0UZ), 1UZ);
        BOOST_CHECK_EQUAL(board, (std::uint64_t{1} << 63U) | std::uint64_t{0b1010'0100});
        BOOST_CHECK(readings_agree_with(board, std::array{board}));
        BOOST_CHECK_EQUAL(squares.max_size(), 64UZ);
}

// A fixed number of blocks is a static width over them: one block, two, and more, each the storage's own path.
BOOST_AUTO_TEST_CASE(AnArrayOfBlocksIsAStaticWidth)
{
        auto one  = std::array<std::uint8_t, 1>{0x81};
        auto two  = std::array<std::uint16_t, 2>{0x0001, 0x8000};
        auto four = std::array<std::uint32_t, 4>{0x0000'0003, 0x0000'0000, 0x8000'0000, 0x0001'0000};

        static_assert(decltype(xstd::bit_span(one))::static_extent == 8UZ);
        static_assert(decltype(xstd::bit_span(two))::static_extent == 32UZ);
        static_assert(decltype(xstd::bit_span(four))::static_extent == 128UZ);
        BOOST_CHECK(readings_agree_with(one, one));
        BOOST_CHECK(readings_agree_with(two, two));
        BOOST_CHECK(readings_agree_with(four, four));

        xstd::bit_span(four).fill(true);
        BOOST_CHECK(std::ranges::equal(four, std::array<std::uint32_t, 4>{~0U, ~0U, ~0U, ~0U}));
        BOOST_CHECK(xstd::bit_span(four).all());
}

// A contiguous range of blocks that does not subscript is no bit storage, so no view lends it.
BOOST_AUTO_TEST_CASE(ARangeThatDoesNotSubscriptIsNotLent)
{
        static_assert(not span_viewable<std::initializer_list<std::uint64_t>&>);
        static_assert(not span_viewable<std::initializer_list<std::uint64_t> const&>);
        BOOST_CHECK(true);
}

// A built-in array is a static width over its blocks, as std::span deduces span<T, N> from T (&)[N].
BOOST_AUTO_TEST_CASE(ABuiltInArrayIsAStaticWidth)
{
        four_blocks blocks = {0x8000'0000'0000'0001ULL, 0x0ULL, 0xF0ULL, 0x8000'0000'0000'0000ULL};
        auto const& view   = blocks;
        auto same          = std::array<std::uint64_t, 4>{0x8000'0000'0000'0001ULL, 0x0ULL, 0xF0ULL, 0x8000'0000'0000'0000ULL};

        static_assert(std::same_as<decltype(xstd::bit_span(blocks)), xstd::bit_span<std::span<std::uint64_t, 4>>>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(blocks)), xstd::bit_set_view<std::span<std::uint64_t, 4>>>);
        static_assert(std::same_as<decltype(xstd::bit_span(view)), xstd::bit_span<std::span<std::uint64_t const, 4>>>);
        static_assert(decltype(xstd::bit_span(blocks))::static_extent == 256UZ);
        static_assert(std::same_as<decltype(xstd::bit_set_view(view)), xstd::bit_set_view<std::span<std::uint64_t const, 4>, 256>>);
        BOOST_CHECK(readings_agree_with(blocks, blocks));
        BOOST_CHECK(readings_agree_with(view, same));
        BOOST_CHECK(std::ranges::equal(xstd::bit_span(blocks), xstd::bit_span(same)));
        BOOST_CHECK_EQUAL(xstd::bit_set_view(blocks).max_size(), 256UZ);
        BOOST_CHECK(std::ranges::equal(xstd::bit_set_view(blocks), xstd::bit_set_view(same)));

        // The views write through to the array, and a const one reads without writing.
        auto const seq = xstd::bit_span(blocks);
        seq[64]        = true;
        BOOST_CHECK_EQUAL(blocks[1], 1U);
        static_assert(not can_assign_element<decltype(xstd::bit_span(view))>);
}

// A growable container's blocks are a run-time width, the whole of them, and never grow through the view.
BOOST_AUTO_TEST_CASE(AVectorOfBlocksIsARunTimeWidth)
{
        auto blocks    = std::vector<std::uint32_t>{1U, 0x8000'0000U, 0U};
        auto const seq = xstd::bit_span(blocks);
        static_assert(sizeof(seq) == sizeof(std::span<std::uint32_t>));
        static_assert(decltype(seq)::static_extent == std::dynamic_extent);
        BOOST_CHECK_EQUAL(seq.size(), 96UZ);
        BOOST_CHECK_EQUAL(seq.max_size(), 96UZ);
        BOOST_CHECK(readings_agree_with(blocks, blocks));

        seq[40] = true;
        BOOST_CHECK_EQUAL(blocks[1], 0x8000'0000U | (1U << 8U));

        // A window reaches across the block boundary and writes only its own positions.
        auto const window = seq.subspan(30, 20);
        BOOST_CHECK(static_cast<bool>(window[10]));
        window.fill(true);
        BOOST_CHECK_EQUAL(blocks[0], 0xC000'0001U);
        BOOST_CHECK_EQUAL(blocks[1], 0x8003'FFFFU);
        BOOST_CHECK(readings_agree_with(blocks, blocks));
        BOOST_CHECK_EQUAL(xstd::bit_set_view(blocks).max_size(), 96UZ);
}

// A span is blocks already lent, so it is handed over as it is, temporary or not.
BOOST_AUTO_TEST_CASE(ASpanIsHandedOverAsItIs)
{
        auto blocks       = std::array<std::uint16_t, 3>{0x0001, 0x0000, 0x8000};
        auto const middle = xstd::bit_span(std::span(blocks).subspan(1, 1));
        BOOST_CHECK_EQUAL(middle.size(), 16UZ);
        middle[3] = true;
        BOOST_CHECK_EQUAL(blocks[1], 0x0008U);

        auto const fixed = xstd::bit_set_view(std::span(blocks).first<2>());
        static_assert(decltype(xstd::bit_span(std::span(blocks).first<2>()))::static_extent == 32UZ);
        BOOST_CHECK(fixed.contains(0UZ) and fixed.contains(19UZ));
        BOOST_CHECK_EQUAL(fixed.size(), 2UZ);
}

// No blocks is width zero, and every reading of it is empty.
BOOST_AUTO_TEST_CASE(NoBlocksIsWidthZero)
{
        auto blocks = std::vector<std::uint64_t>();
        BOOST_CHECK(xstd::bit_set_view(blocks).empty());
        BOOST_CHECK(xstd::bit_set_view(blocks).begin() == xstd::bit_set_view(blocks).end());
        BOOST_CHECK(xstd::bit_span(blocks).empty());
        BOOST_CHECK(xstd::bit_span(blocks).all());
        BOOST_CHECK(xstd::bit_span(blocks).none());
}

// A view over const blocks reads them and cannot write, and one over the same blocks unqualified sees what it writes.
BOOST_AUTO_TEST_CASE(AConstViewReadsAndCannotWrite)
{
        auto blocks    = std::array<std::uint16_t, 2>{0x0100, 0x0000};
        auto const set = xstd::bit_set_view(std::as_const(blocks));
        BOOST_CHECK(set.contains(8UZ));
        BOOST_CHECK_EQUAL(set.size(), 1UZ);
        static_assert(not can_insert<decltype(set)>);
        static_assert(not can_assign_element<decltype(xstd::bit_span(std::as_const(blocks)))>);
        static_assert(can_insert<decltype(xstd::bit_set_view(blocks))>);
        static_assert(can_assign_element<decltype(xstd::bit_span(blocks))>);

        auto const writable = xstd::bit_set_view(blocks);
        BOOST_CHECK(writable.insert(0UZ).second);
        BOOST_CHECK_EQUAL(blocks[0], 0x0101U);
        BOOST_CHECK(set.contains(0UZ));

        auto const board = std::uint64_t{0b110};
        static_assert(not can_insert<decltype(xstd::bit_set_view(board))>);
        BOOST_CHECK(std::ranges::equal(xstd::bit_set_view(board), std::array{1UZ, 2UZ}));
}

// The views are borrowed ranges as std::span is: an iterator refers to the blocks, and outlives the view it came from.
BOOST_AUTO_TEST_CASE(IteratorsOutliveTheView)
{
        auto board = std::uint64_t{0b1001'0000};
        static_assert(std::ranges::view<decltype(xstd::bit_set_view(board))>);
        static_assert(std::ranges::borrowed_range<decltype(xstd::bit_set_view(board))>);

        auto const first = xstd::bit_set_view(board).begin();
        BOOST_CHECK_EQUAL(*first, 4UZ);
        auto const found = std::ranges::find(xstd::bit_set_view(board), 7UZ);
        BOOST_CHECK_EQUAL(*found, 7UZ);

        auto blocks    = std::vector<std::uint8_t>{0x00, 0x00};
        auto const bit = xstd::bit_span(blocks).begin() + 9;
        *bit           = true;
        BOOST_CHECK_EQUAL(blocks[1], 0x02U);
}

// A view is named by the blocks it is handed: a block as itself, a range as the span that lends it.
BOOST_AUTO_TEST_CASE(AViewIsNamedByItsBlocks)
{
        auto board  = std::uint64_t{};
        auto blocks = std::vector<std::uint32_t>(2);
        auto fixed  = std::array<std::uint16_t, 2>{};
        static_assert(std::same_as<decltype(xstd::bit_set_view(board)), xstd::bit_set_view<std::uint64_t>>);
        static_assert(std::same_as<decltype(xstd::bit_set_view(std::as_const(board))), xstd::bit_set_view<std::uint64_t const>>);
        static_assert(std::same_as<decltype(xstd::bit_span(blocks)), xstd::bit_span<std::span<std::uint32_t>>>);
        static_assert(std::same_as<decltype(xstd::bit_span(std::as_const(fixed))), xstd::bit_span<std::span<std::uint16_t const, 2>>>);
        static_assert(std::same_as<decltype(xstd::bit_span(blocks).subspan(1, 2)), xstd::bit_subspan<std::span<std::uint32_t>>>);
        static_assert(std::same_as<decltype(xstd::bit_span(fixed).first<4>()), xstd::bit_subspan<std::span<std::uint16_t, 2>, 4>>);
        BOOST_CHECK(xstd::bit_set_view(board).empty());

        // The deduced views write through to the blocks they are named by.
        auto const seq = xstd::bit_span(blocks);
        seq[33]        = true;
        BOOST_CHECK_EQUAL(blocks[1], 0x02U);
        xstd::bit_span(fixed).first<4>().fill(true);
        BOOST_CHECK_EQUAL(fixed[0], 0x000FU);
}

// Only what outlives the view is viewed: an unsigned block by lvalue, a range of them by lvalue or as a borrowed range.
BOOST_AUTO_TEST_CASE(OnlyLentUnsignedBlocksAreViewed)
{
        static_assert(set_viewable<std::uint8_t&> and set_viewable<std::uint8_t const&>);
        static_assert(span_viewable<std::vector<std::uint32_t>&> and span_viewable<std::vector<std::uint32_t> const&>);
        static_assert(span_viewable<std::span<std::uint32_t>> and span_viewable<std::span<std::uint32_t const, 2>>);
        static_assert(not set_viewable<std::uint8_t>);
        static_assert(not span_viewable<std::vector<std::uint32_t>>);
        static_assert(not span_viewable<std::array<std::uint32_t, 2>>);
        static_assert(not set_viewable<int&>);
        static_assert(not set_viewable<bool&>);
        static_assert(not span_viewable<std::vector<int>&>);
}

BOOST_AUTO_TEST_SUITE_END()
