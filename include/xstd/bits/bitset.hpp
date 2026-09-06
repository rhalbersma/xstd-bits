//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/basic_bitset.hpp>              // basic_bitset
#include <xstd/bits/block_sequence.hpp>            // block_array
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
using bitset = basic_bitset<block_array<Block, N>>;

}       // namespace xstd

#endif // XSTD_BITS_BITSET_HPP
