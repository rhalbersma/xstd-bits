//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP

#include <xstd/bits/bit_valarray/detail/valarray_adaptor.hpp> // valarray_adaptor
#include <xstd/bits/detail/bit_block_container.hpp>           // bit_block_container
#include <xstd/bits/detail/num_blocks.hpp>                    // num_blocks_v, tiling_block
#include <xstd/bits/detail/rebind.hpp>                        // rebind
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <boost/container/new_allocator.hpp>                  // new_allocator
#include <boost/container/small_vector.hpp>                   // small_vector
#include <cstddef>                                            // size_t
#include <memory>                                             // allocator_traits

namespace xstd {

// The packed std::valarray<bool> at a run-time size, inline up to N bits and on the heap past them.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
        requires bits::detail::tiling_block<Block>
class basic_bit_small_valarray : public bits::detail::valarray_adaptor<bits::detail::bit_block_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, basic_bit_small_valarray<Block, N, Alloc>>
{
        using base_type = bits::detail::valarray_adaptor<bits::detail::bit_block_container<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>, basic_bit_small_valarray<Block, N, Alloc>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

template<std::size_t N>
using bit_small_valarray = basic_bit_small_valarray<std::size_t, N>;

// Another block takes the allocator with it, as std::allocator_traits rebinds one; another width keeps it.
template<class Block, std::size_t N, class Alloc>
struct bits::detail::rebind<basic_bit_small_valarray<Block, N, Alloc>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_small_valarray<OtherBlock, N, typename std::allocator_traits<Alloc>::template rebind_alloc<OtherBlock>>;

        template<std::size_t M>
        using with_width = basic_bit_small_valarray<Block, M, Alloc>;
};

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP
