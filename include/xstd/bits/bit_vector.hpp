//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

#include <xstd/bits/bit_sequence_adaptor.hpp>      // bit_sequence_adaptor
#include <xstd/bits/from_bit_storage.hpp>          // from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator
#include <vector>                                  // vector

namespace xstd {

// The sequence reading over a heap of blocks: std::vector<bool> under the name Hinnant proposed for it.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_bit_vector = bit_sequence_adaptor<std::vector<Block, Allocator>>;

using bit_vector = basic_bit_vector<std::size_t>;

// Spelled as the alias spells its storage, so that alias deduction reaches Block and Allocator.
template<xstd::unsigned_integer Block, class Allocator>
bit_sequence_adaptor(from_bit_storage_t, std::vector<Block, Allocator>) -> bit_sequence_adaptor<std::vector<Block, Allocator>>;

template<xstd::unsigned_integer Block, class Allocator>
bit_sequence_adaptor(from_bit_storage_t, std::vector<Block, Allocator>, Allocator) -> bit_sequence_adaptor<std::vector<Block, Allocator>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_VECTOR_HPP
