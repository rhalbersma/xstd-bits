//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP

#include <xstd/bits/bit_valarray/bit_valarray.hpp>     // basic_bit_valarray
#include <xstd/bits/detail/num_blocks.hpp>             // num_blocks_v, tiling_block
#include <xstd/bits/ext/boost/detail/small_blocks.hpp> // bit_storage_rebind
#include <xstd/ints/concepts/unsigned_integer.hpp>     // unsigned_integer
#include <boost/container/new_allocator.hpp>           // new_allocator
#include <boost/container/small_vector.hpp>            // small_vector
#include <cstddef>                                     // size_t

namespace xstd {

// The packed std::valarray<bool> at a run-time size, the blocks of N bits inline and the heap past them.
template<xstd::unsigned_integer Block, std::size_t N, class Alloc = boost::container::new_allocator<Block>>
        requires bits::detail::tiling_block<Block>
using basic_bit_small_valarray = basic_bit_valarray<boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>>;

template<std::size_t N>
using bit_small_valarray = basic_bit_small_valarray<std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_VALARRAY_HPP
