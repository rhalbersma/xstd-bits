//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP
#define XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP

#include <xstd/bits/bit_valarray/detail/valarray_adaptor.hpp> // valarray_adaptor
#include <xstd/bits/detail/bit_block_container.hpp>           // bit_block_container
#include <xstd/bits/detail/num_blocks.hpp>                    // tiling_block
#include <xstd/bits/detail/rebind.hpp>                        // rebind
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t
#include <memory>                                             // allocator, allocator_traits
#include <vector>                                             // vector

namespace xstd {

// The packed std::valarray<bool> over a heap of blocks: every operator elementwise, at a size set at run time.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
        requires bits::detail::tiling_block<Block>
class basic_bit_valarray : public bits::detail::valarray_adaptor<bits::detail::bit_block_container<std::vector<Block, Allocator>>, basic_bit_valarray<Block, Allocator>>
{
        using base_type = bits::detail::valarray_adaptor<bits::detail::bit_block_container<std::vector<Block, Allocator>>, basic_bit_valarray<Block, Allocator>>;

public:
        using base_type::base_type;
        using base_type::operator=;
};

using bit_valarray = basic_bit_valarray<std::size_t>;

// Another block takes the allocator with it, as std::allocator_traits rebinds one; the size is a run-time value.
template<class Block, class Allocator>
struct bits::detail::rebind<basic_bit_valarray<Block, Allocator>>
{
        using block_type = Block;

        template<class OtherBlock>
        using with_block = basic_bit_valarray<OtherBlock, typename std::allocator_traits<Allocator>::template rebind_alloc<OtherBlock>>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_VALARRAY_BIT_VALARRAY_HPP
