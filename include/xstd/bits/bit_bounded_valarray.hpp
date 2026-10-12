//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BOUNDED_VALARRAY_HPP
#define XSTD_BITS_BIT_BOUNDED_VALARRAY_HPP

#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/detail/bounded_blocks.hpp>      // bounded_blocks_for
#include <xstd/bits/detail/num_blocks.hpp>          // num_blocks_v, tiling_block
#include <xstd/bits/detail/rebind.hpp>              // rebind
#include <xstd/bits/detail/valarray_adaptor.hpp>    // valarray_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <cstddef>                                  // size_t

namespace xstd {

// The packed std::valarray<bool> at a run-time size within a capacity of N, past which it throws std::bad_alloc.
template<xstd::unsigned_integer Block, std::size_t N>
        requires bits::detail::tiling_block<Block>
class basic_bit_bounded_valarray : public bits::detail::valarray_adaptor<bits::detail::bit_block_container<bits::detail::bounded_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bit_bounded_valarray<Block, N>>
{
        using base_type = bits::detail::valarray_adaptor<bits::detail::bit_block_container<bits::detail::bounded_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bit_bounded_valarray<Block, N>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

template<std::size_t N>
using bit_bounded_valarray = basic_bit_bounded_valarray<std::size_t, N>;

template<class Block, std::size_t N>
struct bits::detail::rebind<basic_bit_bounded_valarray<Block, N>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_bounded_valarray<OtherBlock, N>;

        template<std::size_t M>
        using with_width = basic_bit_bounded_valarray<Block, M>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_BOUNDED_VALARRAY_HPP
