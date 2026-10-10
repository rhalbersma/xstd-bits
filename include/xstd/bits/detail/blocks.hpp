//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCKS_HPP
#define XSTD_BITS_DETAIL_BLOCKS_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <cstddef>                                         // size_t
#include <span>                                            // span

// What the public views are named by: the blocks, never the storage built over them.
namespace xstd::bits::detail {

// The blocks and width a storage is built over, const where it is: how a guide names a view over existing storage.
template<class Bits>
struct blocks_of;

template<class Blocks, std::size_t N>
struct blocks_of<bit_block_container<Blocks, N>>
{
        using type = Blocks;

        static constexpr std::size_t width = N;
};

template<class Blocks, std::size_t N>
struct blocks_of<bit_block_container<Blocks, N> const>
{
        using type = Blocks const;

        static constexpr std::size_t width = N;
};

// A span's width is the one its blocks name, which is what a view over them defaults to.
template<class Block, std::size_t E, std::size_t N>
struct blocks_of<bit_block_container<std::span<Block, E>, N>>
{
        using type = std::span<Block, E>;

        static constexpr std::size_t width = xstd::bit_blocks_extent_v<type>;
};

template<class Block, std::size_t E, std::size_t N>
struct blocks_of<bit_block_container<std::span<Block, E>, N> const>
{
        using type = std::span<Block const, E>;

        static constexpr std::size_t width = xstd::bit_blocks_extent_v<type>;
};

template<class Bits>
using blocks_of_t = blocks_of<Bits>::type;

template<class Bits>
inline constexpr std::size_t blocks_width_v = blocks_of<Bits>::width;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCKS_HPP
