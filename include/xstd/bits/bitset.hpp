//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/bitset_adaptor.hpp>              // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/memory.hpp>                      // align_up
#include <cstddef>                                   // size_t
#include <limits>                                    // digits

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open. [design.md#the-public-names]
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = bitset_adaptor<detail::bits::contiguous_bit_array<Block, N>>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

// The width rounded up to whole blocks, as the other two static names offer: no unused tail, so every block is the value. [design.md#the-public-names]
namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = xstd::basic_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

}       // namespace aligned
}       // namespace xstd

#endif // XSTD_BITS_BITSET_HPP
