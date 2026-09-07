//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/bitset_adaptor.hpp>            // bitset_adaptor
#include <xstd/bits/block_sequence.hpp>            // block_vector
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart. [design.md#the-idempotent-wrapper]
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_dynamic_bitset = bitset_adaptor<block_vector<Block, Allocator>>;

using dynamic_bitset = basic_dynamic_bitset<std::size_t>;

}       // namespace xstd

#endif  // XSTD_BITS_DYNAMIC_BITSET_HPP
