//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_EXPAND_BLOCKS_HPP
#define XSTD_BITS_DETAIL_EXPAND_BLOCKS_HPP

#include <xstd/bits/detail/bit_stream.hpp>         // bit_stream
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/expanded.hpp>           // expanded
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd::bits::detail {

// The positions each mask selects in dst take the next bits of src in order, as f combines them with what dst holds.
template<class Src, class Masks, xstd::unsigned_integer Block, class F>
constexpr auto expand_blocks(Src const& src, Masks const& masks, borrowed_bits<Block>& dst, F f) noexcept
        -> void
{
        auto stream = bit_stream<Block>(src.blocks());
        masks.for_each([&](std::size_t k, Block mask, std::size_t count) noexcept -> void {
                dst[k] = f(dst[k], expanded(stream.take(count), mask), mask);
        });
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_EXPAND_BLOCKS_HPP
