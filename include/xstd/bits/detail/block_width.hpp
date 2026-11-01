//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_WIDTH_HPP
#define XSTD_BITS_DETAIL_BLOCK_WIDTH_HPP

#include <xstd/bits/detail/bit_blocks_extent.hpp>  // bit_blocks_extent_v
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cstddef>                                 // size_t
#include <type_traits>                             // conditional_t

namespace xstd::bits::detail {

template<xstd::unsigned_integer Block>
inline constexpr std::size_t block_width = xstd::bit_blocks_extent_v<Block>;

template<xstd::unsigned_integer Block>
inline constexpr auto block_ones = xstd::numeric_limits<Block>::max();

// A block held at least a word wide, so a narrow one leaves no padding beside the counts it is stored with.
template<xstd::unsigned_integer Block>
using wide_block_t = std::conditional_t<(sizeof(Block) < sizeof(std::size_t)), std::size_t, Block>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCK_WIDTH_HPP
