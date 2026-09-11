//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/set_adaptor.hpp>               // set_adaptor
#include <xstd/bits/detail/block_vector.hpp>       // block_vector
#include <xstd/bits/ownership.hpp>                 // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator

namespace xstd {

// The set reading over a heap of blocks: the flagship, benchmarked against std::set, and the one name without a qualifier. [design.md#the-public-names]
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_bit_set = set_adaptor<block_vector<Block, Allocator>, ownership::owns>;

using bit_set = basic_bit_set<std::size_t>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SET_HPP
