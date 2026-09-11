//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/bitset_adaptor.hpp>            // bitset_adaptor
#include <xstd/bits/detail/block_array.hpp>        // block_array
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/memory.hpp>                    // align_up
#include <cstddef>                                 // size_t
#include <limits>                                  // digits

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block>
using basic_bitset = bitset_adaptor<detail::bits::block_array<Block, N>>;

template<std::size_t N>
using bitset = basic_bitset<N, std::size_t>;

// The width rounded up to whole blocks, as the other two static names offer: no unused tail, so every block is the value. [design.md#the-public-names]
namespace aligned {

template<std::size_t N, xstd::unsigned_integer Block>
using basic_bitset = xstd::basic_bitset<xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), Block>;

template<std::size_t N>
using bitset = basic_bitset<N, std::size_t>;

}       // namespace aligned
}       // namespace xstd

#endif // XSTD_BITS_BITSET_HPP
