//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

#include <xstd/bits/basic_bit_sequence.hpp>        // basic_bit_sequence
#include <xstd/bits/block_sequence.hpp>            // block_vector
#include <xstd/bits/ownership.hpp>                 // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator

namespace xstd {

// The sequence reading over a heap of blocks: std::vector<bool> under the name Hinnant proposed for it. [design.md#the-public-names]
template<xstd::unsigned_integer Block = std::size_t, class Allocator = std::allocator<Block>>
using bit_vector = basic_bit_sequence<block_vector<Block, Allocator>, ownership::owns, false>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_VECTOR_HPP
