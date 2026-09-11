//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <memory>                                        // allocator
#include <vector>                                        // vector

namespace xstd::detail::bits {

// The second vehicle: a width on the heap, growing as a set of positions does. [design.md#the-one-vehicle]
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using contiguous_bit_vector = contiguous_bit_container<std::vector<Block, Allocator>>;

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP
