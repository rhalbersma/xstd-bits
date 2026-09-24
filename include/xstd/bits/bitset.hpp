//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/bitset_adaptor.hpp>                  // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <limits>                                        // numeric_limits

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = bitset_adaptor<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>;

// Written over the adaptor rather than basic_bitset, as MSVC deduces through one alias here but not through two.
template<std::size_t N>
using bitset = bitset_adaptor<std::array<std::size_t, bits::detail::num_blocks_v<std::size_t, N>>, N>;

// The width rounded up to whole blocks: no unused tail, so every block is the value.
namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = xstd::basic_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

} // namespace aligned

} // namespace xstd

#endif // XSTD_BITS_BITSET_HPP
