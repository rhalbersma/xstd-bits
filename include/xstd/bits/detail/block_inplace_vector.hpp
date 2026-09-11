//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_INPLACE_VECTOR_HPP
#define XSTD_BITS_DETAIL_BLOCK_INPLACE_VECTOR_HPP

#include <version>                                         // IWYU pragma: keep; __cpp_lib_inplace_vector

// The third vehicle: a run-time width under a compile-time capacity of N bits. Behind the feature macro until
// every library in the matrix has it, which is why the guard lives here and not beside the other two --
// a consumer of block_array pays nothing for a conditional that concerns only this one. [design.md#the-inplace-column]
#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/detail/block_sequence.hpp>             // block_sequence, num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <cstddef>                                         // size_t
#include <inplace_vector>                                  // inplace_vector

namespace xstd::detail::bits {

template<xstd::unsigned_integer Block, std::size_t N>
using block_inplace_vector = block_sequence<std::inplace_vector<Block, num_blocks_v<Block, N>>>;

}       // namespace xstd::detail::bits

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_DETAIL_BLOCK_INPLACE_VECTOR_HPP
