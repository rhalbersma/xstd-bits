//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_NUM_BLOCKS_HPP
#define XSTD_BITS_DETAIL_NUM_BLOCKS_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <xstd/ints/cstdlib/div_ceil.hpp>                  // div_ceil
#include <bit>                                             // has_single_bit
#include <concepts>                                        // same_as
#include <cstddef>                                         // size_t
#include <limits>                                          // numeric_limits
#include <type_traits>                                     // remove_cv_t

namespace xstd::bits::detail {

// The whole blocks that N bits take, none at width zero, at any block width, a power of two or not.
template<xstd::unsigned_integer Block, std::size_t N>
inline constexpr std::size_t num_blocks_v = xstd::div_ceil(N, static_cast<std::size_t>(xstd::bit_blocks_extent_v<Block>)).quotient;

// N rounded up to a whole number of blocks, the width those blocks hold.
template<xstd::unsigned_integer Block, std::size_t N>
inline constexpr std::size_t whole_blocks_width_v = num_blocks_v<Block, N> * xstd::bit_blocks_extent_v<Block>;

// A block an owner holds by value, so cv-unqualified, as a standard container's element type must be.
template<class Block>
concept owned_block = xstd::unsigned_integer<Block> and std::same_as<Block, std::remove_cv_t<Block>>;

// A block that tiles any width: a power of two of bits, a byte or more, so no position or byte straddles two blocks.
template<class Block>
concept tiling_block =
        owned_block<Block> and
        std::has_single_bit(xstd::bit_blocks_extent_v<Block>) and
        xstd::bit_blocks_extent_v<Block> >= static_cast<std::size_t>(std::numeric_limits<unsigned char>::digits);

// Any other width, 17 or 127 bits, holds one fixed-width value: N positions within a single block.
template<class Block, std::size_t N>
concept holds_width = tiling_block<Block> or (owned_block<Block> and N <= xstd::bit_blocks_extent_v<Block>);

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_NUM_BLOCKS_HPP
