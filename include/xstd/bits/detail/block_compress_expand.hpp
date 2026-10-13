//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_COMPRESS_EXPAND_HPP
#define XSTD_BITS_DETAIL_BLOCK_COMPRESS_EXPAND_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/borrowed_bits.hpp>              // borrowed_bits
#include <xstd/bits/detail/intrin.hpp>                     // popcount
#include <xstd/bits/detail/shift.hpp>                      // partial_block_mask, shl
#include <xstd/ints/bit/bit_compress.hpp>                  // bit_compress
#include <xstd/ints/bit/bit_expand.hpp>                    // bit_expand
#include <xstd/ints/bit/bit_repeat.hpp>                    // bit_repeat
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <xstd/ints/limits.hpp>                            // numeric_limits
#include <algorithm>                                       // min
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint64_t
#include <ranges>                                          // iota
#include <span>                                            // span

// Gather and scatter over a range of blocks: P3104's bit_compress and bit_expand a block at a time, packed by block_at.
namespace xstd::bits::detail {

template<xstd::unsigned_integer Block>
inline constexpr std::size_t block_width = xstd::bit_blocks_extent_v<Block>;

template<xstd::unsigned_integer Block>
inline constexpr auto block_ones = xstd::numeric_limits<Block>::max();

// A full mask selects the whole block in place, which no permutation needs to find out.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto compressed(Block src, Block mask) noexcept
        -> Block
{
        return mask == block_ones<Block> ? src : xstd::bit_compress(src, mask);
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto expanded(Block src, Block mask) noexcept
        -> Block
{
        return mask == block_ones<Block> ? src : xstd::bit_expand(src, mask);
}

// The masks of a slice whose stride is below the block width, one per block from the one holding its start.
template<xstd::unsigned_integer Block>
class slice_masks
{
        static constexpr auto digits = block_width<Block>;

        std::size_t m_first;
        std::size_t m_last;
        std::size_t m_final;
        std::size_t m_stride;
        std::size_t m_carry;
        std::size_t m_base;
        std::size_t m_offset;
        std::size_t m_below;

public:
        // The stride selects at least one position in every block from the first to the last.
        static constexpr bool has_empty_blocks = false;

        [[nodiscard]] constexpr slice_masks(std::size_t start, std::size_t size, std::size_t stride) noexcept
                : m_first(start / digits)
                , m_last(((start + ((size - 1UZ) * stride)) / digits) + 1UZ)
                , m_final(start + ((size - 1UZ) * stride))
                , m_stride(stride)
                , m_carry(digits % stride)
                , m_base(m_first * digits)
                , m_offset((start - m_base) % stride)
                , m_below(start - m_base)
        {}

        [[nodiscard]] constexpr auto first() const noexcept
                -> std::size_t
        {
                return m_first;
        }

        [[nodiscard]] constexpr auto last() const noexcept
                -> std::size_t
        {
                return m_last;
        }

        // One bit every stride, shifted to the block's first position, cut below the start and above the last one.
        [[nodiscard]] constexpr auto next() noexcept
                -> Block
        {
                auto const kept   = std::ranges::min(digits, m_final - m_base + 1UZ);
                auto const ones   = xstd::bit_repeat(Block{1}, static_cast<int>(m_stride));
                auto const period = static_cast<Block>(shl(ones, m_offset) & static_cast<Block>(~partial_block_mask<Block>(m_below)));
                m_base += digits;
                m_below  = 0UZ;
                m_offset = m_offset >= m_carry ? m_offset - m_carry : m_offset + m_stride - m_carry;
                return static_cast<Block>(period & partial_block_mask<Block>(kept));
        }
};

// The masks a mask array or an ascending gslice stores, one per block, any of which may select nothing.
template<xstd::unsigned_integer Block>
class stored_masks
{
        std::span<Block const> m_masks;
        std::size_t m_next = 0UZ;

public:
        static constexpr bool has_empty_blocks = true;

        [[nodiscard]] constexpr explicit stored_masks(std::span<Block const> masks) noexcept
                : m_masks(masks)
        {}

        [[nodiscard]] static constexpr auto first() noexcept
                -> std::size_t
        {
                return 0UZ;
        }

        [[nodiscard]] constexpr auto last() const noexcept
                -> std::size_t
        {
                return m_masks.size();
        }

        [[nodiscard]] constexpr auto next() noexcept
                -> Block
        {
                return m_masks[m_next++];
        }
};

// The masks a bit_gslice stores as 64-bit words, read a block at a time at any block width; a block may select nothing.
template<xstd::unsigned_integer Block>
class word_masks
{
        static constexpr auto digits = block_width<Block>;
        static constexpr auto word   = 64UZ;

        std::span<std::uint64_t const> m_words;
        std::size_t m_last;
        std::size_t m_pos = 0UZ;

public:
        static constexpr bool has_empty_blocks = true;

        // The blocks that hold the first width positions.
        [[nodiscard]] constexpr word_masks(std::span<std::uint64_t const> words, std::size_t width) noexcept
                : m_words(words)
                , m_last((width + digits - 1UZ) / digits)
        {}

        [[nodiscard]] static constexpr auto first() noexcept
                -> std::size_t
        {
                return 0UZ;
        }

        [[nodiscard]] constexpr auto last() const noexcept
                -> std::size_t
        {
                return m_last;
        }

        [[nodiscard]] constexpr auto next() noexcept
                -> Block
        {
                auto mask = Block{};
                if constexpr (digits <= word) {
                        // A block of a power of two of at most a word lies inside one word, which every block below width has.
                        mask = static_cast<Block>(m_words[m_pos / word] >> (m_pos % word));
                } else {
                        // A wider block spans words, the last of which may lie past width.
                        for (auto const i : std::views::iota(0UZ, digits / word)) {
                                auto const index = (m_pos / word) + i;
                                auto const bits  = index < m_words.size() ? static_cast<Block>(m_words[index]) : Block{};
                                mask             = static_cast<Block>(mask | shl(bits, i * word));
                        }
                }
                m_pos += digits;
                return mask;
        }
};

// What a scatter makes of a destination block d, the bits deposited under the mask m, and m: only m's positions change.
inline constexpr auto scatter_assign = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(static_cast<Block>(d & static_cast<Block>(~m)) | e); };

inline constexpr auto scatter_and = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(d & static_cast<Block>(e | static_cast<Block>(~m))); };

inline constexpr auto scatter_or = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d | e); };

inline constexpr auto scatter_xor = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d ^ e); };

// The bits each mask selects from the blocks of src, packed in order into dst from position zero.
template<class Src, class Masks, xstd::unsigned_integer Block>
constexpr auto compress_blocks(Src const& src, Masks masks, borrowed_bits<Block>& dst) noexcept
        -> void
{
        auto pos = 0UZ;
        for (auto const k : std::views::iota(masks.first(), masks.last())) {
                auto const mask  = masks.next();
                auto const count = detail::popcount(mask);
                if constexpr (Masks::has_empty_blocks) {
                        // An empty block writes nothing, and its position may lie one past the last block of dst.
                        if (count == 0UZ) {
                                continue;
                        }
                }
                dst.block_at(pos, compressed(src[k], mask), partial_block_mask<Block>(count));
                pos += count;
        }
}

// The positions each mask selects in dst take the next bits of src in order, as f combines them with what dst holds.
template<class Src, class Masks, xstd::unsigned_integer Block, class F>
constexpr auto expand_blocks(Src const& src, Masks masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto pos = 0UZ;
        for (auto const k : std::views::iota(masks.first(), masks.last())) {
                auto const mask = masks.next();
                if constexpr (Masks::has_empty_blocks) {
                        // An empty block reads nothing, and its position may lie one past the last block of src.
                        if (mask == Block{}) {
                                continue;
                        }
                }
                dst[k] = f(dst[k], expanded(src.block_at(pos), mask), mask);
                pos += detail::popcount(mask);
        }
}

// A scalar at every position the masks select, as f combines it with what dst holds.
template<class Masks, xstd::unsigned_integer Block, class F>
constexpr auto fill_blocks(bool value, Masks masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const splat = value ? block_ones<Block> : Block{};
        for (auto const k : std::views::iota(masks.first(), masks.last())) {
                auto const mask = masks.next();
                dst[k]          = f(dst[k], static_cast<Block>(mask & splat), mask);
        }
}

// One position's bit through a block combiner, as a mask of that one position.
template<xstd::unsigned_integer Block, class F>
[[nodiscard]] constexpr auto combined(F f, bool d, bool e) noexcept
        -> bool
{
        return static_cast<Block>(f(static_cast<Block>(static_cast<unsigned>(d)), static_cast<Block>(static_cast<unsigned>(e)), Block{1}) & Block{1}) != Block{};
}

// One position's bit written with no branch on its value, which random bits would mispredict.
template<xstd::unsigned_integer Block>
constexpr auto put(borrowed_bits<Block>& dst, std::size_t pos, bool value) noexcept
        -> void
{
        auto& block      = dst[pos / block_width<Block>];
        auto const mask  = shl(Block{1}, pos % block_width<Block>);
        auto const splat = static_cast<Block>(Block{} - static_cast<Block>(static_cast<unsigned>(value)));
        block            = static_cast<Block>(static_cast<Block>(block & static_cast<Block>(~mask)) | static_cast<Block>(mask & splat));
}

// The bits at index(0), ..., index(count - 1) of src, in that order, into dst from position zero.
template<class Src, xstd::unsigned_integer Block, class Index>
constexpr auto gather_indices(Src const& src, std::size_t count, Index index, borrowed_bits<Block>& dst) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, count)) {
                put(dst, j, src.test(index(j)));
        }
}

// One position at a time and in order, so a position named twice takes what f makes of its later bit.
template<class Src, xstd::unsigned_integer Block, class Index, class F>
constexpr auto expand_indices(Src const& src, std::size_t count, Index index, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, count)) {
                auto const pos = index(j);
                put(dst, pos, combined<Block>(f, dst.test(pos), src.test(j)));
        }
}

template<xstd::unsigned_integer Block, class Index, class F>
constexpr auto fill_indices(bool value, std::size_t count, Index index, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, count)) {
                auto const pos = index(j);
                put(dst, pos, combined<Block>(f, dst.test(pos), value));
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCK_COMPRESS_EXPAND_HPP
