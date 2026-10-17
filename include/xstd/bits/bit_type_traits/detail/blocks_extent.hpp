//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_DETAIL_BLOCKS_EXTENT_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_DETAIL_BLOCKS_EXTENT_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <cstddef>                                 // size_t
#include <span>                                    // dynamic_extent, span

// The width a cv-unqualified storage type names, which the public trait asks of the storage with its cv stripped.
namespace xstd::bits::detail {

template<class Bits>
inline constexpr std::size_t blocks_extent_v = std::dynamic_extent;

template<xstd::unsigned_integer Block>
inline constexpr std::size_t blocks_extent_v<Block> = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits);

template<xstd::unsigned_integer Block, std::size_t K>
inline constexpr std::size_t blocks_extent_v<std::array<Block, K>> = blocks_extent_v<Block> * K;

template<xstd::unsigned_integer Block, std::size_t E>
        requires (E != std::dynamic_extent)
inline constexpr std::size_t blocks_extent_v<std::span<Block, E>> = blocks_extent_v<Block> * E;

// A built-in array names its bound as std::array does; its cv sits on the element and is stripped there.
template<xstd::unsigned_integer Block, std::size_t K>
inline constexpr std::size_t blocks_extent_v<Block[K]> = blocks_extent_v<Block> * K; // NOLINT(modernize-avoid-c-arrays): a built-in array is what it names.

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_TYPE_TRAITS_DETAIL_BLOCKS_EXTENT_HPP
