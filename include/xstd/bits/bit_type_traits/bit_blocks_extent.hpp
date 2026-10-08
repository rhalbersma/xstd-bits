//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp>  // bit_block
#include <xstd/bits/bit_concepts/bit_blocks.hpp> // bit_blocks
#include <xstd/ints/limits.hpp>                  // numeric_limits
#include <array>                                 // array
#include <cstddef>                               // size_t
#include <span>                                  // dynamic_extent, span
#include <type_traits>                           // remove_cv_t

namespace xstd {

// The width bit storage names by its type: every bit of a block or of a fixed number of blocks, else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_extent_v = std::dynamic_extent;

template<bit_blocks Bits>
        requires bit_block<Bits>
inline constexpr std::size_t bit_blocks_extent_v<Bits> = static_cast<std::size_t>(xstd::numeric_limits<Bits>::digits);

template<bit_block Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K>> = bit_blocks_extent_v<Block> * K;

template<bit_block Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K> const> = bit_blocks_extent_v<std::array<Block, K>>;

template<bit_block Block, std::size_t E>
        requires (E != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_extent_v<std::span<Block, E>> = bit_blocks_extent_v<Block> * E;

// A built-in array names its bound as std::array does, over blocks of any constness.
template<class Block, std::size_t K>
        requires bit_block<Block>
inline constexpr std::size_t bit_blocks_extent_v<Block[K]> = bit_blocks_extent_v<std::remove_cv_t<Block>> * K; // NOLINT(modernize-avoid-c-arrays): a built-in array is what it names.

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP
