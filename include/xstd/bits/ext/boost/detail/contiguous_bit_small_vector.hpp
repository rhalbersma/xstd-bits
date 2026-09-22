//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_DETAIL_CONTIGUOUS_BIT_SMALL_VECTOR_HPP
#define XSTD_BITS_EXT_BOOST_DETAIL_CONTIGUOUS_BIT_SMALL_VECTOR_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/small_vector.hpp>              // small_vector
#include <cstddef>                                       // size_t

namespace xstd::detail::bits {

// The fourth vehicle, from outside the standard library: inline up to a capacity of N bits, on the heap past it.
template<xstd::unsigned_integer Block, std::size_t N, class Allocator>
using contiguous_bit_small_vector =
        contiguous_bit_container<boost::container::small_vector<Block, num_blocks_v<Block, N>, Allocator>>;

} // namespace xstd::detail::bits

#endif // XSTD_BITS_EXT_BOOST_DETAIL_CONTIGUOUS_BIT_SMALL_VECTOR_HPP
