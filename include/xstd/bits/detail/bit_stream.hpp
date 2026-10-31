//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_STREAM_HPP
#define XSTD_BITS_DETAIL_BIT_STREAM_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_width, wide_block_t
#include <xstd/bits/detail/shift.hpp>              // shl, shr
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <span>                                    // span

namespace xstd::bits::detail {

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

        // The next count bits in the low ones of a block, garbage above, which every caller masks away.
        [[nodiscard]] constexpr auto take(std::size_t count) noexcept
                -> Block
        {
                auto bits = static_cast<Block>(m_buffer);
                if (count <= m_avail) {
                        m_buffer = shr_up_to_width(bits, count);
                        m_avail -= count;
                        return bits;
                }
                // A count past what is held reaches into the next block, which a count within the source always has.
                auto const next = m_blocks[m_next++];
                bits            = static_cast<Block>(bits | shl(next, m_avail));
                m_buffer        = shr_up_to_width(next, count - m_avail);
                m_avail         = block_width<Block> - (count - m_avail);
                return bits;
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_STREAM_HPP
