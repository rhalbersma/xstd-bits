//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

#include <xstd/bits/bit_set_adaptor.hpp>           // bit_set_adaptor
#include <xstd/bits/from_bit_storage.hpp>          // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator
#include <vector>                                  // vector

namespace xstd {

// The set reading over a heap of blocks: the flagship, and the one name without a qualifier.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_bit_set = bit_set_adaptor<std::vector<Block, Allocator>>;

using bit_set = basic_bit_set<std::size_t>;

// Spelled as the alias spells its storage, so that alias deduction reaches Block and Allocator.
template<xstd::unsigned_integer Block, class Allocator>
bit_set_adaptor(from_bit_storage_t, std::vector<Block, Allocator>) -> bit_set_adaptor<std::vector<Block, Allocator>>;

template<xstd::unsigned_integer Block, class Allocator>
bit_set_adaptor(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> bit_set_adaptor<std::vector<Block, Allocator>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_HPP
