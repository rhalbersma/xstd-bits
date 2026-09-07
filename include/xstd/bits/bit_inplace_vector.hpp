//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_HPP

#include <version>                                         // IWYU pragma: keep; __cpp_lib_inplace_vector

// The column comes and goes with its storage, and an alias withholds a name rather than a capability. [design.md#the-inplace-column]
#ifdef __cpp_lib_inplace_vector
#include <xstd/bits/sequence_adaptor.hpp>                  // sequence_adaptor
#include <xstd/bits/block_sequence.hpp>                    // block_inplace_vector
#include <xstd/bits/ownership.hpp>                         // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <cstddef>                                         // size_t

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write, named after the container it packs. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block>
using basic_bit_inplace_vector = sequence_adaptor<block_inplace_vector<Block, N>, ownership::owns, false>;

template<std::size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<N, std::size_t>;

}       // namespace xstd

#endif  // __cpp_lib_inplace_vector

#endif  // XSTD_BITS_BIT_INPLACE_VECTOR_HPP
