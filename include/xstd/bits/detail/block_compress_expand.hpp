//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_COMPRESS_EXPAND_HPP
#define XSTD_BITS_DETAIL_BLOCK_COMPRESS_EXPAND_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/borrowed_bits.hpp>              // borrowed_bits
#include <xstd/bits/detail/intrin.hpp>                     // popcount
#include <xstd/bits/detail/shift.hpp>                      // partial_block_mask, shl, shr
#include <xstd/ints/bit/bit_compress.hpp>                  // bit_compress
#include <xstd/ints/bit/bit_expand.hpp>                    // bit_expand
#include <xstd/ints/bit/bit_repeat.hpp>                    // bit_repeat
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <xstd/ints/limits.hpp>                            // numeric_limits
#include <algorithm>                                       // copy, min, transform
#include <array>                                           // array
#include <concepts>                                        // same_as
#include <cstddef>                                         // size_t
#include <cstdint>                                         // uint64_t
#include <numeric>                                         // gcd
#include <ranges>                                          // iota
#include <span>                                            // span
#include <type_traits>                                     // conditional_t, remove_const_t, remove_cvref_t

// Gather and scatter over a range of blocks: P3104's bit_compress and bit_expand a block at a time, packed by block_at.
namespace xstd::bits::detail {

template<xstd::unsigned_integer Block>
inline constexpr std::size_t block_width = xstd::bit_blocks_extent_v<Block>;

template<xstd::unsigned_integer Block>
inline constexpr auto block_ones = xstd::numeric_limits<Block>::max();

// The condition under which xstd-ints runs bit_compress and bit_expand as PEXT and PDEP, which permute a full mask as fast.
#if defined(__BMI2__) || (defined(_MSC_VER) && !defined(__clang__) && defined(__AVX2__))
inline constexpr bool permutes_in_hardware = true;
#else
inline constexpr bool permutes_in_hardware = false;
#endif

// A full mask selects the whole block in place, which the portable kernels would take the long way round to find.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto compressed(Block src, Block mask) noexcept
        -> Block
{
        if constexpr (permutes_in_hardware) {
                return xstd::bit_compress(src, mask);
        } else {
                return mask == block_ones<Block> ? src : xstd::bit_compress(src, mask);
        }
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto expanded(Block src, Block mask) noexcept
        -> Block
{
        if constexpr (permutes_in_hardware) {
                return xstd::bit_expand(src, mask);
        } else {
                return mask == block_ones<Block> ? src : xstd::bit_expand(src, mask);
        }
}

// The masks of a slice whose stride is below the block width, one per block from the one holding its start.
template<xstd::unsigned_integer Block>
class slice_masks
{
        static constexpr auto digits = block_width<Block>;

        // Held at least a word wide, so a narrow block leaves no padding between the counts.
        using wide_block = std::conditional_t<(sizeof(Block) < sizeof(std::size_t)), std::size_t, Block>;

        std::size_t m_first;
        std::size_t m_last;
        std::size_t m_next;
        std::size_t m_period;
        std::size_t m_phase = 0UZ;
        // The cuts below the start and above the final position, made once.
        wide_block m_head;
        wide_block m_tail;
        // The masks of one period: block k's offset falls back by digits % stride, so they repeat every stride / gcd blocks.
        std::array<wide_block, digits> m_masks{};

public:
        // The stride selects at least one position in every block from the first to the last.
        static constexpr bool has_empty_blocks = false;

        [[nodiscard]] constexpr slice_masks(std::size_t start, std::size_t size, std::size_t stride) noexcept
                : m_first(start / digits)
                , m_last(((start + ((size - 1UZ) * stride)) / digits) + 1UZ)
                , m_next(m_first)
                , m_period(stride / std::gcd(stride, digits))
                , m_head(static_cast<Block>(~partial_block_mask<Block>(start % digits)))
                , m_tail(partial_block_mask<Block>(((start + ((size - 1UZ) * stride)) % digits) + 1UZ))
        {
                auto const ones  = xstd::bit_repeat(Block{1}, static_cast<int>(stride));
                auto const carry = digits % stride;
                auto offset      = (start % digits) % stride;
                for (auto const k : std::views::iota(0UZ, m_period)) {
                        m_masks[k] = shl(ones, offset);
                        offset     = offset >= carry ? offset - carry : offset + stride - carry;
                }
        }

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

        // The period's next mask, cut at the first and the last block alone.
        [[nodiscard]] constexpr auto next() noexcept
                -> Block
        {
                auto mask = static_cast<Block>(m_masks[m_phase]);
                if (m_next == m_first) {
                        mask = static_cast<Block>(mask & static_cast<Block>(m_head));
                }
                if (m_next + 1UZ == m_last) {
                        mask = static_cast<Block>(mask & static_cast<Block>(m_tail));
                }
                ++m_next;
                m_phase = m_phase + 1UZ == m_period ? 0UZ : m_phase + 1UZ;
                return mask;
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
constexpr auto compress_blocks(Src const& src, Masks&& masks, borrowed_bits<Block>& dst) noexcept
        -> void
{
        // The packed bits gather in a register and each block of dst is stored once, full, and never loaded.
        auto out  = 0UZ;
        auto fill = 0UZ;
        auto acc  = Block{};
        for (auto const k : std::views::iota(masks.first(), masks.last())) {
                auto const mask   = masks.next();
                auto const packed = compressed(src[k], mask);
                auto const before = fill;
                acc               = static_cast<Block>(acc | shl(packed, before));
                fill += detail::popcount(mask);
                if (fill >= block_width<Block>) {
                        dst[out++] = acc;
                        fill -= block_width<Block>;
                        // What did not fit, shifted in two steps, so a shift by the whole block width is never asked for.
                        acc = shr(shr(packed, 1UZ), block_width<Block> - before - 1UZ);
                }
        }
        if (fill != 0UZ) {
                dst[out] = acc;
        }
}

// The positions each mask selects in dst take the next bits of src in order, as f combines them with what dst holds.
template<class Src, class Masks, xstd::unsigned_integer Block, class F>
constexpr auto expand_blocks(Src const& src, Masks&& masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto pos = 0UZ;
        for (auto const k : std::views::iota(masks.first(), masks.last())) {
                auto const mask = masks.next();
                if constexpr (std::remove_cvref_t<Masks>::has_empty_blocks) {
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
constexpr auto fill_blocks(bool value, Masks&& masks, borrowed_bits<Block>& dst, F f) noexcept
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

// A run of size positions from start, the slice of stride one: a block at a time through block_at, no permutation.
template<class Src, xstd::unsigned_integer Block>
constexpr auto gather_run(Src const& src, std::size_t start, std::size_t size, borrowed_bits<Block>& dst) noexcept
        -> void
{
        // From a block boundary every whole block is a plain copy into the clear dst, at memmove's speed.
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        std::ranges::copy(src.blocks().subspan(start / block_width<Block>, whole), dst.blocks().begin());
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                dst.block_at(j, src.block_at(start + j), partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j)));
        }
}

template<class Src, xstd::unsigned_integer Block, class F>
constexpr auto expand_run(Src const& src, std::size_t start, std::size_t size, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        auto const from  = src.blocks().first(whole);
        auto const onto  = dst.blocks().subspan(start / block_width<Block>, whole);
        if constexpr (std::same_as<F, std::remove_const_t<decltype(scatter_assign)>>) {
                // An assignment of whole blocks is a copy, at memmove's speed; onto themselves, as v[s] = v, there is nothing to do.
                if (from.data() != onto.data()) {
                        std::ranges::copy(from, onto.begin());
                }
        } else {
                std::ranges::transform(onto, from, onto.begin(), [&](Block d, Block e) noexcept -> Block { return f(d, e, block_ones<Block>); });
        }
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                auto const mask = partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j));
                auto const pos  = start + j;
                dst.block_at(pos, f(dst.block_at(pos), static_cast<Block>(src.block_at(j) & mask), mask), mask);
        }
}

template<xstd::unsigned_integer Block, class F>
constexpr auto fill_run(bool value, std::size_t start, std::size_t size, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const splat = value ? block_ones<Block> : Block{};
        auto const whole = start % block_width<Block> == 0UZ ? size / block_width<Block> : 0UZ;
        for (auto const k : std::views::iota(0UZ, whole)) {
                auto& block = dst[(start / block_width<Block>)+k];
                block       = f(block, splat, block_ones<Block>);
        }
        for (auto j = whole * block_width<Block>; j < size; j += block_width<Block>) {
                auto const mask = partial_block_mask<Block>(std::ranges::min(block_width<Block>, size - j));
                auto const pos  = start + j;
                dst.block_at(pos, f(dst.block_at(pos), static_cast<Block>(splat & mask), mask), mask);
        }
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
