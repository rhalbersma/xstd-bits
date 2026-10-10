//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCKS_HPP
#define XSTD_BITS_DETAIL_BLOCKS_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <xstd/bits/detail/borrowed_bits.hpp>              // block_span_t, borrowable_block, borrowable_blocks, borrowed_bits
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <cstddef>                                         // size_t
#include <span>                                            // span
#include <type_traits>                                     // conditional_t, is_const_v, remove_const_t, remove_reference_t

// What the public views are named by: the blocks, never the storage built over them.
namespace xstd::bits::detail {

template<class T, class Bits>
using const_as_t = std::conditional_t<std::is_const_v<T>, Bits const, Bits>;

// A view's storage: someone else's blocks by span, or the storage of an owner over the same blocks it refers into.
template<class Blocks, std::size_t N>
struct view_storage_for
{
        using type = const_as_t<Blocks, bit_block_container<std::remove_const_t<Blocks>, N>>;
};

template<xstd::unsigned_integer Block, std::size_t N>
struct view_storage_for<Block, N>
{
        using type = const_as_t<Block, borrowed_bits<std::remove_const_t<Block>, 1>>;
};

template<class Block, std::size_t E, std::size_t N>
struct view_storage_for<std::span<Block, E>, N>
{
        using type = const_as_t<Block, borrowed_bits<std::remove_const_t<Block>, E>>;
};

template<class Blocks, std::size_t N>
using view_storage_t = view_storage_for<Blocks, N>::type;

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

// The blocks a guide deduces from what it is handed: a block as itself, a range as the span that lends it.
template<class W>
struct lent_blocks;

template<borrowable_block W>
struct lent_blocks<W>
{
        using type = std::remove_reference_t<W>;
};

template<borrowable_blocks W>
struct lent_blocks<W>
{
        using type = block_span_t<std::remove_reference_t<W>>;
};

template<class W>
using lent_blocks_t = lent_blocks<W>::type;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCKS_HPP
