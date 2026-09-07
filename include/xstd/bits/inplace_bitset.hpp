//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_INPLACE_BITSET_HPP
#define XSTD_BITS_INPLACE_BITSET_HPP

#include <version>                                         // IWYU pragma: keep; __cpp_lib_inplace_vector

// The column comes and goes with its storage, and an alias withholds a name rather than a capability. [design.md#the-inplace-column]
#ifdef __cpp_lib_inplace_vector
#include <xstd/bits/bitset_adaptor.hpp>                    // bitset_adaptor
#include <xstd/bits/block_sequence.hpp>                    // block_inplace_vector
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <cstddef>                                         // size_t

namespace xstd {

// A resizable bitset that never allocates, which is what embedded code asks for; no bit_ prefix, bitset already carrying the word. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block>
using basic_inplace_bitset = bitset_adaptor<block_inplace_vector<Block, N>>;

template<std::size_t N>
using inplace_bitset = basic_inplace_bitset<N, std::size_t>;

}       // namespace xstd

#endif  // __cpp_lib_inplace_vector

#endif  // XSTD_BITS_INPLACE_BITSET_HPP
