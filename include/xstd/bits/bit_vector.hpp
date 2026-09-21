//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

#include <xstd/bits/basic_bits.hpp>                   // basic_bits, sequence_reading_tag, vector_container_tag
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <xstd/bits/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <cstddef>                                    // size_t
#include <memory>                                     // allocator
#include <span>                                       // dynamic_extent

namespace xstd {

// The sequence reading over a heap of blocks: std::vector<bool> under the name Hinnant proposed for it.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using basic_bit_vector = basic_bits<sequence_reading_tag, vector_container_tag, Block, std::dynamic_extent, Allocator>;

using bit_vector = basic_bit_vector<std::size_t>;

} // namespace xstd

#endif // XSTD_BITS_BIT_VECTOR_HPP
