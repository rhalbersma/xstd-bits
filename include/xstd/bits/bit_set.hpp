//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/basic_bit_set.hpp>             // basic_bit_set
#include <xstd/bits/block_sequence.hpp>            // block_vector
#include <xstd/bits/ownership.hpp>                 // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator

namespace xstd {

// The set reading over a heap of blocks: the flagship, benchmarked against std::set, and the one name without a qualifier. [design.md#the-public-names]
template<xstd::unsigned_integer Block = std::size_t, class Allocator = std::allocator<Block>>
using bit_set = basic_bit_set<block_vector<Block, Allocator>, ownership::owns>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SET_HPP
