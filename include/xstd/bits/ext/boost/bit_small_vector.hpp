//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_VECTOR_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_VECTOR_HPP

#include <xstd/bits/bit_sequence_adaptor.hpp>            // bit_sequence_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <boost/container/new_allocator.hpp>             // new_allocator
#include <boost/container/small_vector.hpp>              // small_vector
#include <cstddef>                                       // size_t

namespace xstd {

// The sequence reading over the small-vector column; the allocator is Boost's own, as that container defaults to it.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
using basic_bit_small_vector = bit_sequence_adaptor<boost::container::small_vector<Block, detail::bits::num_blocks_v<Block, N>, Alloc>>;

template<std::size_t N>
using bit_small_vector = basic_bit_small_vector<std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_VECTOR_HPP
