//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_DETAIL_SMALL_BLOCKS_HPP
#define XSTD_BITS_EXT_BOOST_DETAIL_SMALL_BLOCKS_HPP

#include <xstd/bits/bit_type_traits/bit_storage_rebind.hpp> // bit_storage_rebind
#include <boost/container/small_vector.hpp>                 // small_vector
#include <cstddef>                                          // size_t
#include <memory>                                           // allocator_traits

// boost::container::small_vector as a bit storage: K blocks inline, and the heap past them.
namespace xstd {

// The inline blocks are its count; a void allocator is Boost's default, and stays void under another block.
template<class Block, std::size_t K, class Allocator>
struct bit_storage_rebind<boost::container::small_vector<Block, K, Allocator>>
{
        static constexpr std::size_t block_count = K;

        template<class OtherBlock>
        using with_block = boost::container::small_vector<OtherBlock, K, typename std::allocator_traits<Allocator>::template rebind_alloc<OtherBlock>>;

        template<std::size_t L>
        using with_block_count = boost::container::small_vector<Block, L, Allocator>;
};

template<class Block, std::size_t K>
struct bit_storage_rebind<boost::container::small_vector<Block, K>>
{
        static constexpr std::size_t block_count = K;

        template<class OtherBlock>
        using with_block = boost::container::small_vector<OtherBlock, K>;

        template<std::size_t L>
        using with_block_count = boost::container::small_vector<Block, L>;
};

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_DETAIL_SMALL_BLOCKS_HPP
