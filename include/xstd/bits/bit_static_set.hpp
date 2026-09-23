//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STATIC_SET_HPP
#define XSTD_BITS_BIT_STATIC_SET_HPP

#include <xstd/bits/bit_set_adaptor.hpp>                 // bit_set_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <limits>                                        // numeric_limits

namespace xstd {

// The static set: the basic name leaves the block open, the restricted one is the machine word.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_static_set = bit_set_adaptor<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_static_set = xstd::basic_bit_static_set<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<std::size_t, N>;

} // namespace aligned

} // namespace xstd

#endif // XSTD_BITS_BIT_STATIC_SET_HPP
