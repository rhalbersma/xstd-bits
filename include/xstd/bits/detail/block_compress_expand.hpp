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
#include <type_traits>                                     // conditional_t, remove_const_t

// Gather and scatter over a range of blocks: P3104's bit_compress and bit_expand a block at a time, packed by block_at.
namespace xstd::bits::detail {

template<xstd::unsigned_integer Block>
inline constexpr std::size_t block_width = xstd::bit_blocks_extent_v<Block>;

template<xstd::unsigned_integer Block>
inline constexpr auto block_ones = xstd::numeric_limits<Block>::max();

// Where xstd-ints runs bit_compress and bit_expand as PEXT and PDEP, which permute a full mask as fast as any other.
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

// A block held at least a word wide, so a narrow one leaves no padding beside the counts it is stored with.
template<xstd::unsigned_integer Block>
using wide_block_t = std::conditional_t<(sizeof(Block) < sizeof(std::size_t)), std::size_t, Block>;

// The masks of a slice whose stride is below the block width, one per block from the one holding its start.
template<xstd::unsigned_integer Block>
class slice_masks
{
        static constexpr auto digits = block_width<Block>;

        using wide_block = wide_block_t<Block>;

        std::size_t m_first;
        std::size_t m_last;
        std::size_t m_period;
        // Where the whole periods of the middle blocks end, and the remainder begins.
        std::size_t m_periods_end;
        // The first block's mask, cut below the start, and the last block's, cut above the final position.
        wide_block m_first_mask;
        wide_block m_last_mask;
        // The middle blocks' masks over one period from the second block on: they repeat every stride / gcd blocks.
        std::array<wide_block, digits> m_masks{};

public:
        [[nodiscard]] constexpr slice_masks(std::size_t start, std::size_t size, std::size_t stride) noexcept
                : m_first(start / digits)
                , m_last(((start + ((size - 1UZ) * stride)) / digits) + 1UZ)
                , m_period(stride / std::gcd(stride, digits))
                , m_periods_end(m_first + 1UZ + (((m_last - m_first - std::ranges::min(m_last - m_first, 2UZ)) / m_period) * m_period))
                , m_first_mask()
                , m_last_mask()
        {
                auto const ones  = xstd::bit_repeat(Block{1}, static_cast<int>(stride));
                auto const carry = digits % stride;
                auto const tail  = partial_block_mask<Block>(((start + ((size - 1UZ) * stride)) % digits) + 1UZ);
                auto offset      = (start % digits) % stride;
                auto const step  = [&] noexcept -> void { offset = offset >= carry ? offset - carry : offset + stride - carry; };
                m_first_mask     = static_cast<Block>(shl(ones, offset) & static_cast<Block>(~partial_block_mask<Block>(start % digits)));
                if (m_first + 1UZ == m_last) {
                        m_first_mask = static_cast<Block>(static_cast<Block>(m_first_mask) & tail);
                }
                step();
                for (auto const p : std::views::iota(0UZ, m_period)) {
                        m_masks[p] = shl(ones, offset);
                        step();
                }
                m_last_mask = static_cast<Block>(static_cast<Block>(m_masks[(m_last - m_first - 2UZ) % m_period]) & tail);
        }

        // f(k, mask) for every block the slice touches, in order; the middle in whole periods, with no phase to keep.
        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                f(m_first, static_cast<Block>(m_first_mask));
                if (m_first + 1UZ == m_last) {
                        return;
                }
                auto k = m_first + 1UZ;
                if (m_period == 1UZ) {
                        // A stride dividing the block width: one mask for every middle block.
                        for (auto const constant = static_cast<Block>(m_masks[0]); k + 1UZ < m_last; ++k) {
                                f(k, constant);
                        }
                        f(k, static_cast<Block>(m_last_mask));
                        return;
                }
                for (; k < m_periods_end; k += m_period) {
                        for (auto const p : std::views::iota(0UZ, m_period)) {
                                f(k + p, static_cast<Block>(m_masks[p]));
                        }
                }
                for (auto p = 0UZ; k + 1UZ < m_last; ++k, ++p) {
                        f(k, static_cast<Block>(m_masks[p]));
                }
                f(m_last - 1UZ, static_cast<Block>(m_last_mask));
        }
};

// The masks a mask array stores, one per block.
template<xstd::unsigned_integer Block>
class stored_masks
{
        std::span<Block const> m_masks;

public:
        [[nodiscard]] constexpr explicit stored_masks(std::span<Block const> masks) noexcept
                : m_masks(masks)
        {}

        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                for (auto const k : std::views::iota(0UZ, m_masks.size())) {
                        f(k, m_masks[k]);
                }
        }
};

// The masks a bit_gslice stores as 64-bit words, read a block at a time at any block width.
template<xstd::unsigned_integer Block>
class word_masks
{
        static constexpr auto digits = block_width<Block>;
        static constexpr auto word   = 64UZ;

        std::span<std::uint64_t const> m_words;
        std::size_t m_last;

public:
        // The blocks that hold the first width positions.
        [[nodiscard]] constexpr word_masks(std::span<std::uint64_t const> words, std::size_t width) noexcept
                : m_words(words)
                , m_last((width + digits - 1UZ) / digits)
        {}

        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                for (auto const k : std::views::iota(0UZ, m_last)) {
                        auto const pos = k * digits;
                        if constexpr (digits <= word) {
                                // A block of a power of two of at most a word lies inside one word, which every block below width has.
                                f(k, static_cast<Block>(m_words[pos / word] >> (pos % word)));
                        } else {
                                // A wider block spans words, the last of which may lie past width.
                                auto mask = Block{};
                                for (auto const i : std::views::iota(0UZ, digits / word)) {
                                        auto const index = (pos / word) + i;
                                        auto const bits  = index < m_words.size() ? static_cast<Block>(m_words[index]) : Block{};
                                        mask             = static_cast<Block>(mask | shl(bits, i * word));
                                }
                                f(k, mask);
                        }
                }
        }
};

// A shift right by any count up to the whole block, in two halves, so neither is ever the whole block width.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto shr_up_to_width(Block block, std::size_t n) noexcept
        -> Block
{
        return shr(shr(block, n / 2UZ), n - (n / 2UZ));
}

// The blocks of src read in order as one stream of bits, a register holding what is not yet taken.
template<xstd::unsigned_integer Block>
class bit_stream
{
        std::span<Block const> m_blocks;
        std::size_t m_next  = 0UZ;
        std::size_t m_avail = 0UZ;
        wide_block_t<Block> m_buffer{};

public:
        [[nodiscard]] constexpr explicit bit_stream(std::span<Block const> blocks) noexcept
                : m_blocks(blocks)
        {}

        // The next count bits in the low ones of a block, garbage above them, which every caller masks or deposits away.
        [[nodiscard]] constexpr auto take(std::size_t count) noexcept
                -> Block
        {
                auto const buffer = static_cast<Block>(m_buffer);
                if (count <= m_avail) {
                        m_buffer = shr_up_to_width(buffer, count);
                        m_avail -= count;
                        return buffer;
                }
                // A count past what is held reaches into the next block, which a count within the source always has.
                auto const next = m_blocks[m_next++];
                auto const bits = static_cast<Block>(buffer | shl(next, m_avail));
                m_buffer        = shr_up_to_width(next, count - m_avail);
                m_avail         = block_width<Block> - (count - m_avail);
                return bits;
        }
};

// What a scatter makes of a destination block d, the bits deposited under the mask m, and m: only m's positions change.
inline constexpr auto scatter_assign = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(static_cast<Block>(d & static_cast<Block>(~m)) | e); };

inline constexpr auto scatter_and = []<class Block>(Block d, Block e, Block m) noexcept -> Block { return static_cast<Block>(d & static_cast<Block>(e | static_cast<Block>(~m))); };

inline constexpr auto scatter_or = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d | e); };

inline constexpr auto scatter_xor = []<class Block>(Block d, Block e, Block /* m */) noexcept -> Block { return static_cast<Block>(d ^ e); };

// The bits each mask selects from the blocks of src, packed in order into dst from position zero.
template<class Src, class Masks, xstd::unsigned_integer Block>
constexpr auto compress_blocks(Src const& src, Masks const& masks, borrowed_bits<Block>& dst) noexcept
        -> void
{
        // The packed bits gather in a register and each block of dst is stored once, full, and never loaded.
        auto out  = 0UZ;
        auto fill = 0UZ;
        auto acc  = Block{};
        masks.for_each([&](std::size_t k, Block mask) noexcept -> void {
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
        });
        if (fill != 0UZ) {
                dst[out] = acc;
        }
}

// The positions each mask selects in dst take the next bits of src in order, as f combines them with what dst holds.
template<class Src, class Masks, xstd::unsigned_integer Block, class F>
constexpr auto expand_blocks(Src const& src, Masks const& masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto stream = bit_stream<Block>(src.blocks());
        masks.for_each([&](std::size_t k, Block mask) noexcept -> void {
                dst[k] = f(dst[k], expanded(stream.take(detail::popcount(mask)), mask), mask);
        });
}

// A scalar at every position the masks select, as f combines it with what dst holds.
template<class Masks, xstd::unsigned_integer Block, class F>
constexpr auto fill_blocks(bool value, Masks const& masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto const splat = value ? block_ones<Block> : Block{};
        masks.for_each([&](std::size_t k, Block mask) noexcept -> void { dst[k] = f(dst[k], static_cast<Block>(mask & splat), mask); });
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
