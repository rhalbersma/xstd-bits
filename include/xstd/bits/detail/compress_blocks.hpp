//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_COMPRESS_BLOCKS_HPP
#define XSTD_BITS_DETAIL_COMPRESS_BLOCKS_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/compressed.hpp>         // compressed
#include <xstd/bits/detail/shift.hpp>              // shl, shr
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd::bits::detail {

// The bits each mask selects from the blocks of src, packed in order into dst from position zero.
template<class Src, class Masks, xstd::unsigned_integer Block>
constexpr auto compress_blocks(Src const& src, Masks const& masks, borrowed_bits<Block>& dst) noexcept
        -> void
{
        // The packed bits gather in a register and each block of dst is stored once, full, and never loaded.
        auto out  = 0UZ;
        auto fill = 0UZ;
        auto acc  = Block{};
        masks.for_each([&](std::size_t k, Block mask, std::size_t count) noexcept -> void {
                auto const packed = compressed(src[k], mask);
                auto const before = fill;
                acc               = static_cast<Block>(acc | shl(packed, before));
                fill += count;
                if (fill >= block_width<Block>) {
                        dst[out++] = acc;
                        fill -= block_width<Block>;
                        // What did not fit, shifted in two steps, so no shift is by the whole block width.
                        acc = shr(shr(packed, 1UZ), block_width<Block> - before - 1UZ);
                }
        });
        if (fill != 0UZ) {
                dst[out] = acc;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_COMPRESS_BLOCKS_HPP
