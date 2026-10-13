//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/block_compress_expand.hpp> // compress_blocks, expand_blocks, scatter_and, scatter_assign, scatter_or, scatter_xor, slice_masks, stored_masks
#include <xstd/bits/detail/borrowed_bits.hpp>         // borrowed_bits
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_TEST_CONTEXT
#include <bit>                                        // popcount
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint64_t, uint8_t
#include <limits>                                     // numeric_limits
#include <ranges>                                     // iota
#include <span>                                       // span
#include <utility>                                    // pair
#include <vector>                                     // vector

BOOST_AUTO_TEST_SUITE(Detail)
BOOST_AUTO_TEST_SUITE(BlockCompressExpand)

namespace {

using bools = std::vector<bool>;

template<class Block>
inline constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Block>::digits);

template<class Block>
[[nodiscard]] auto bools_of(std::vector<Block> const& blocks)
        -> bools
{
        auto result = bools(blocks.size() * digits<Block>);
        for (auto const i : std::views::iota(0UZ, result.size())) {
                result[i] = ((blocks[i / digits<Block>] >> (i % digits<Block>)) & Block{1}) != Block{};
        }
        return result;
}

// The oracle a bit at a time: the selected bits of src packed from position zero, the rest of dst untouched.
[[nodiscard]] auto compress_oracle(bools const& src, bools const& mask, bools dst)
        -> bools
{
        auto pos = 0UZ;
        for (auto const i : std::views::iota(0UZ, src.size())) {
                if (mask[i]) {
                        dst[pos++] = src[i];
                }
        }
        return dst;
}

// The oracle for an assigning scatter: the selected positions of dst take the bits of src in order.
[[nodiscard]] auto expand_oracle(bools const& src, bools const& mask, bools dst)
        -> bools
{
        auto pos = 0UZ;
        for (auto const i : std::views::iota(0UZ, dst.size())) {
                if (mask[i]) {
                        dst[i] = src[pos++];
                }
        }
        return dst;
}

// The source and destination patterns, so a kept bit, a written bit and an untouched one all show.
template<class Block>
[[nodiscard]] auto fills(std::size_t num_blocks)
        -> std::vector<std::vector<Block>>
{
        auto alternating = std::vector<Block>(num_blocks);
        auto scrambled   = std::vector<Block>(num_blocks);
        auto state       = std::uint64_t{0x9E37'79B9'7F4A'7C15};
        for (auto const k : std::views::iota(0UZ, num_blocks)) {
                state ^= state << 13U;
                state ^= state >> 7U;
                state ^= state << 17U;
                alternating[k] = static_cast<Block>(0x5555'5555'5555'5555ULL);
                scrambled[k]   = static_cast<Block>(state);
        }
        return {std::vector<Block>(num_blocks), std::vector<Block>(num_blocks, static_cast<Block>(~Block{})), alternating, scrambled};
}

template<class Block>
auto check_compress_expand(std::vector<Block> const& mask_blocks)
        -> void
{
        auto const mask = bools_of(mask_blocks);
        auto const fill = fills<Block>(mask_blocks.size());
        // A scrambled source into an alternating destination, and a full one into a clear one.
        for (auto const& [src_blocks, dst_blocks] : {std::pair(fill[3], fill[2]), std::pair(fill[1], fill[0])}) {
                auto const src = bools_of(src_blocks);
                auto const dst = bools_of(dst_blocks);

                auto source     = src_blocks;
                auto compressed = std::vector<Block>(dst_blocks.size());
                auto const from = xstd::bits::detail::borrowed_bits<Block>(std::span(source));
                auto into       = xstd::bits::detail::borrowed_bits<Block>(std::span(compressed));
                xstd::bits::detail::compress_blocks(from, xstd::bits::detail::stored_masks<Block>(mask_blocks), into);
                BOOST_CHECK(bools_of(compressed) == compress_oracle(src, mask, bools(dst.size())));

                auto expanded = dst_blocks;
                auto onto     = xstd::bits::detail::borrowed_bits<Block>(std::span(expanded));
                xstd::bits::detail::expand_blocks(from, xstd::bits::detail::stored_masks<Block>(mask_blocks), onto, xstd::bits::detail::scatter_assign);
                BOOST_CHECK(bools_of(expanded) == expand_oracle(src, mask, dst));
        }
}

} // namespace

// Every mask over two narrow blocks, so each compressed block lands at every offset across the boundary.
BOOST_AUTO_TEST_CASE(CompressAndExpandAgreeWithTheOracleForEveryMaskOverTwoBlocks)
{
        for (auto const m : std::views::iota(0UZ, 1UZ << 16U)) {
                BOOST_TEST_CONTEXT("mask " << m)
                {
                        check_compress_expand(std::vector<std::uint8_t>{static_cast<std::uint8_t>(m), static_cast<std::uint8_t>(m >> 8U)});
                }
        }
}

// A sweep of masks over wide blocks, each block's count different, so the output offset walks through every residue.
BOOST_AUTO_TEST_CASE(CompressAndExpandAgreeWithTheOracleOverASweepOfWideBlocks)
{
        auto state = std::uint64_t{0xD1B5'4A32'D192'ED03};
        for ([[maybe_unused]] auto const trial : std::views::iota(0UZ, 256UZ)) {
                auto masks = std::vector<std::uint64_t>(5UZ);
                for (auto& mask : masks) {
                        state ^= state << 13U;
                        state ^= state >> 7U;
                        state ^= state << 17U;
                        mask = state & (state >> (state % 7U));
                }
                check_compress_expand(masks);
        }
}

// A slice's masks, block by block, select exactly start + j * stride for j < size, at every start, stride and size.
BOOST_AUTO_TEST_CASE(SliceMasksSelectEveryStrideFromEveryStart)
{
        using Block  = std::uint8_t;
        auto const n = 5UZ * digits<Block>;
        for (auto const start : std::views::iota(0UZ, n)) {
                for (auto const stride : std::views::iota(1UZ, digits<Block>)) {
                        for (auto const size : std::views::iota(1UZ, ((n - 1UZ - start) / stride) + 2UZ)) {
                                BOOST_TEST_CONTEXT("slice(" << start << ", " << size << ", " << stride << ")")
                                {
                                        auto expected = bools(n);
                                        for (auto const j : std::views::iota(0UZ, size)) {
                                                expected[start + (j * stride)] = true;
                                        }
                                        auto blocks = std::vector<Block>(n / digits<Block>);
                                        xstd::bits::detail::slice_masks<Block>(start, size, stride).for_each([&](std::size_t k, Block mask, std::size_t count) -> void {
                                                blocks[k] = mask;
                                                BOOST_CHECK_EQUAL(count, static_cast<std::size_t>(std::popcount(mask)));
                                        });
                                        BOOST_CHECK(bools_of(blocks) == expected);
                                }
                        }
                }
        }
}

// The compound scatters change only the selected positions, each by its own operator.
BOOST_AUTO_TEST_CASE(TheCombinersChangeOnlyTheMaskedPositions)
{
        using Block = std::uint8_t;
        for (auto const d : std::views::iota(0U, 256U)) {
                for (auto const m : std::views::iota(0U, 256U)) {
                        auto const db = static_cast<Block>(d);
                        auto const mb = static_cast<Block>(m);
                        auto const eb = static_cast<Block>(0xA5U & m);
                        BOOST_CHECK(xstd::bits::detail::scatter_and(db, eb, mb) == static_cast<Block>((d & ~m) | (d & eb)));
                        BOOST_CHECK(xstd::bits::detail::scatter_or(db, eb, mb) == static_cast<Block>(d | eb));
                        BOOST_CHECK(xstd::bits::detail::scatter_xor(db, eb, mb) == static_cast<Block>(d ^ eb));
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
