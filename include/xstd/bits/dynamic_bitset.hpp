//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DYNAMIC_BITSET_HPP
#define XSTD_BITS_DYNAMIC_BITSET_HPP

#include <xstd/bits/bitset_adaptor.hpp>            // bitset_adaptor
#include <xstd/bits/from_bit_storage.hpp>          // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator
#include <vector>                                  // vector

namespace xstd {

// The bitset reading over a heap of blocks, boost::dynamic_bitset being its counterpart.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_dynamic_bitset = bitset_adaptor<std::vector<Block, Allocator>>;

using dynamic_bitset = basic_dynamic_bitset<std::size_t>;

// Spelled as the alias spells its storage, so that alias deduction reaches Block and Allocator.
template<xstd::unsigned_integer Block, class Allocator>
bitset_adaptor(from_bit_storage_t, std::vector<Block, Allocator>) -> bitset_adaptor<std::vector<Block, Allocator>>;

template<xstd::unsigned_integer Block, class Allocator>
bitset_adaptor(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> bitset_adaptor<std::vector<Block, Allocator>>;

} // namespace xstd

#endif // XSTD_BITS_DYNAMIC_BITSET_HPP
