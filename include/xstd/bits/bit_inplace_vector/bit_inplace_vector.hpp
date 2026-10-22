//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP

#include <xstd/bits/bit_vector/bit_vector.hpp>     // basic_bit_vector
#include <xstd/bits/detail/inplace_blocks.hpp>     // inplace_blocks_for
#include <xstd/bits/detail/num_blocks.hpp>         // num_blocks_v, tiling_block
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write: N bits in place, in the blocks that hold them.
template<xstd::unsigned_integer Block, std::size_t N>
        requires bits::detail::tiling_block<Block>
using basic_bit_inplace_vector = basic_bit_vector<bits::detail::inplace_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>;

template<std::size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP
