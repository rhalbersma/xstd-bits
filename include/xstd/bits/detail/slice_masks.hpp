//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SLICE_MASKS_HPP
#define XSTD_BITS_DETAIL_SLICE_MASKS_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_width, wide_block_t
#include <xstd/bits/detail/intrinsic.hpp>          // popcount
#include <xstd/bits/detail/shift.hpp>              // partial_block_mask, shl
#include <xstd/ints/bit/bit_repeat.hpp>            // bit_repeat
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // min
#include <array>                                   // array
#include <cassert>                                 // assert
#include <cstddef>                                 // size_t
#include <numeric>                                 // gcd
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// The masks of a slice whose stride is below the block width, one per block from the one holding its start.
template<xstd::unsigned_integer Block>
class slice_masks
{
        static constexpr auto digits = block_width<Block>;

        using wide_block = wide_block_t<Block>;

        std::size_t m_first;
        std::size_t m_last;
        // The table's length: the most whole periods of stride / gcd(stride, digits) blocks that digits entries hold.
        std::size_t m_span;
        // Where the whole table spans of the middle blocks end, and the remainder begins.
        std::size_t m_spans_end;
        // The first block's mask, cut below the start, and the last block's, cut above the final position.
        wide_block m_first_mask;
        wide_block m_last_mask;
        // The middle blocks' masks from the second block on, over whole periods, so a long inner loop runs over them.
        std::array<wide_block, digits> m_masks{};
        // Their popcounts, so a kernel asks none per block.
        std::array<std::size_t, digits> m_counts{};

        // A stride of zero or of a whole block has no period below the block width to tabulate.
        [[nodiscard]] static constexpr auto span_of(std::size_t stride) noexcept
                -> std::size_t
        {
                assert(0UZ < stride and stride < digits);
                auto const period = stride / std::gcd(stride, digits);
                return (digits / period) * period;
        }

public:
        [[nodiscard]] constexpr slice_masks(std::size_t start, std::size_t size, std::size_t stride) noexcept
                : m_first(start / digits)
                , m_last(((start + ((size - 1UZ) * stride)) / digits) + 1UZ)
                , m_span(span_of(stride))
                , m_spans_end(m_first + 1UZ + (((m_last - m_first - std::ranges::min(m_last - m_first, 2UZ)) / m_span) * m_span))
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
                for (auto const p : std::views::iota(0UZ, m_span)) {
                        m_masks[p]  = shl(ones, offset);
                        m_counts[p] = detail::popcount(static_cast<Block>(m_masks[p]));
                        step();
                }
                m_last_mask = static_cast<Block>(static_cast<Block>(m_masks[(m_last - m_first - 2UZ) % m_span]) & tail);
        }

        // f(k, mask, popcount(mask)) for each block the slice touches: edges cut, the middle over the table.
        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                auto const edge = [&](std::size_t k, wide_block mask) noexcept -> void { f(k, static_cast<Block>(mask), detail::popcount(static_cast<Block>(mask))); };
                edge(m_first, m_first_mask);
                if (m_first + 1UZ == m_last) {
                        return;
                }
                auto k = m_first + 1UZ;
                if (m_span == digits) {
                        // A stride dividing the block width has a period of one block: one mask and count.
                        for (auto const constant = static_cast<Block>(m_masks[0]); k + 1UZ < m_last; ++k) {
                                f(k, constant, m_counts[0]);
                        }
                } else {
                        for (; k < m_spans_end; k += m_span) {
                                for (auto const p : std::views::iota(0UZ, m_span)) {
                                        f(k + p, static_cast<Block>(m_masks[p]), m_counts[p]);
                                }
                        }
                        for (auto p = 0UZ; k + 1UZ < m_last; ++k, ++p) {
                                f(k, static_cast<Block>(m_masks[p]), m_counts[p]);
                        }
                }
                edge(m_last - 1UZ, m_last_mask);
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_SLICE_MASKS_HPP
