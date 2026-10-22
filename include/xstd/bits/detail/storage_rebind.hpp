//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_STORAGE_REBIND_HPP
#define XSTD_BITS_DETAIL_STORAGE_REBIND_HPP

#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp>  // bit_blocks_extent_v
#include <xstd/bits/bit_type_traits/bit_storage_rebind.hpp> // bit_storage_rebind
#include <xstd/bits/detail/bit_block_container.hpp>         // admits_owner_extent
#include <xstd/bits/detail/bit_blocks_capacity.hpp>         // bit_blocks_capacity_v
#include <xstd/bits/detail/num_blocks.hpp>                  // num_blocks_v
#include <cstddef>                                          // size_t
#include <ranges>                                           // range_value_t
#include <span>                                             // dynamic_extent

// An owner over a storage and a cap in bits, rebound through the storage's own bit_storage_rebind.
namespace xstd::bits::detail {

// The most bits an owner over Blocks holds: the cap N where one is given, else the blocks' constant capacity, if any.
template<class Blocks, std::size_t N>
inline constexpr std::size_t storage_width_v = N != std::dynamic_extent ? N : bit_blocks_capacity_v<Blocks>;

// A cap only under a constant capacity, and in its last block: a heap of blocks bounds no width of its own.
template<class Blocks, std::size_t N>
concept fits_storage = admits_owner_extent<Blocks, storage_width_v<Blocks, N>>();

template<class Blocks>
concept rebindable_storage = requires { typename bit_storage_rebind<Blocks>::template with_block<std::ranges::range_value_t<Blocks>>; };

template<class Blocks>
concept counted_storage =
        rebindable_storage<Blocks> and
        requires {
                bit_storage_rebind<Blocks>::block_count;
                typename bit_storage_rebind<Blocks>::template with_block_count<bit_storage_rebind<Blocks>::block_count>;
        };

template<class Blocks, class OtherBlock>
using storage_with_block_t = bit_storage_rebind<Blocks>::template with_block<OtherBlock>;

template<class Blocks, std::size_t L>
using storage_with_block_count_t = bit_storage_rebind<Blocks>::template with_block_count<L>;

// The owner over other blocks and another cap: each owner over a storage names itself here, as type<OtherBlocks, M>.
template<class Owner>
struct storage_owner
{};

template<class Owner, class OtherBlocks, std::size_t M>
using storage_owner_t = storage_owner<Owner>::template type<OtherBlocks, M>;

// The members rebind asks of an owner over Blocks capped at N: none where the storage cannot change its block.
template<class Owner, class Blocks, std::size_t N>
struct storage_rebind
{};

// A storage of no fixed count changes its block alone, and keeps the cap.
template<class Owner, class Blocks, std::size_t N>
        requires rebindable_storage<Blocks> and (not counted_storage<Blocks>)
struct storage_rebind<Owner, Blocks, N>
{
        using block_type = std::ranges::range_value_t<Blocks>;

        template<class OtherBlock>
        using with_block = storage_owner_t<Owner, storage_with_block_t<Blocks, OtherBlock>, N>;
};

// A counted storage's width is the cap, else its blocks' bits; another block or width counts the blocks that hold it.
template<class Owner, class Blocks, std::size_t N>
        requires counted_storage<Blocks>
struct storage_rebind<Owner, Blocks, N>
{
        using block_type                   = std::ranges::range_value_t<Blocks>;
        static constexpr std::size_t width = N != std::dynamic_extent ? N : bit_storage_rebind<Blocks>::block_count * bit_blocks_extent_v<block_type>;

        template<class OtherBlock>
        using with_block = storage_owner_t<Owner, storage_with_block_count_t<storage_with_block_t<Blocks, OtherBlock>, num_blocks_v<OtherBlock, width>>, N>;

        template<std::size_t M>
        using with_width = storage_owner_t<Owner, storage_with_block_count_t<Blocks, num_blocks_v<block_type, M>>, N == std::dynamic_extent ? std::dynamic_extent : M>;
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_STORAGE_REBIND_HPP
