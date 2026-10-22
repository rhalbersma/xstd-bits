//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_BIT_INPLACE_VALARRAY_HPP
#define XSTD_BITS_BIT_VALARRAY_BIT_INPLACE_VALARRAY_HPP

#include <xstd/bits/bit_valarray/bit_valarray.hpp> // basic_bit_valarray
#include <xstd/bits/detail/inplace_blocks.hpp>     // inplace_blocks_for
#include <xstd/bits/detail/num_blocks.hpp>         // num_blocks_v, tiling_block
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd {

// The packed std::valarray<bool> at a run-time size within a capacity of N, past which it throws std::bad_alloc.
template<xstd::unsigned_integer Block, std::size_t N>
        requires bits::detail::tiling_block<Block>
using basic_bit_inplace_valarray = basic_bit_valarray<bits::detail::inplace_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>;

template<std::size_t N>
using bit_inplace_valarray = basic_bit_inplace_valarray<std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_BIT_VALARRAY_BIT_INPLACE_VALARRAY_HPP
