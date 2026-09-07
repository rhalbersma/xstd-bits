//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/basic_bitset.hpp>              // basic_bitset
#include <xstd/bits/block_sequence.hpp>            // block_vector
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart. [design.md#the-idempotent-wrapper]
template<xstd::unsigned_integer Block = std::size_t, class Allocator = std::allocator<Block>>
using dynamic_bitset = basic_bitset<block_vector<Block, Allocator>>;

}       // namespace xstd

#endif  // XSTD_BITS_DYNAMIC_BITSET_HPP
