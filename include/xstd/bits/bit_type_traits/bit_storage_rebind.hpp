//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_STORAGE_REBIND_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_STORAGE_REBIND_HPP

#include <xstd/misc/concepts/simple_allocator.hpp> // simple_allocator
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator_traits

// How a bit storage changes its block, and, where its type carries a count of blocks, that count.
namespace xstd {

// Specialized beside a storage of another shape: with_block<OtherBlock>, and block_count with with_block_count<L>.
template<class Blocks>
struct bit_storage_rebind
{};

// A sequence over an allocator, as std::vector: the allocator rebinds with the block, as std::allocator_traits does.
template<template<class, class> class Storage, class Block, class Allocator>
        requires xstd::simple_allocator<Allocator>
struct bit_storage_rebind<Storage<Block, Allocator>>
{
        template<class OtherBlock>
        using with_block = Storage<OtherBlock, typename std::allocator_traits<Allocator>::template rebind_alloc<OtherBlock>>;
};

// A sequence of at most K blocks in place, as std::inplace_vector.
template<template<class, std::size_t> class Storage, class Block, std::size_t K>
struct bit_storage_rebind<Storage<Block, K>>
{
        static constexpr std::size_t block_count = K;

        template<class OtherBlock>
        using with_block = Storage<OtherBlock, K>;

        template<std::size_t L>
        using with_block_count = Storage<Block, L>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_STORAGE_REBIND_HPP
