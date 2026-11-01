//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_COMPRESSED_HPP
#define XSTD_BITS_DETAIL_COMPRESSED_HPP

#include <xstd/bits/detail/block_width.hpp>          // block_ones
#include <xstd/bits/detail/permutes_in_hardware.hpp> // permutes_in_hardware
#include <xstd/ints/bit/bit_compress.hpp>            // bit_compress
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer

namespace xstd::bits::detail {

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

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_COMPRESSED_HPP
