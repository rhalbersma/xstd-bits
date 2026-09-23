//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

#include <xstd/bits/bit_sequence_adaptor.hpp>            // bit_sequence_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/memory.hpp>                          // align_up
#include <array>                                         // array
#include <cstddef>                                       // size_t
#include <limits>                                        // numeric_limits

namespace xstd {

// The packed std::array<bool, N>, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = bit_sequence_adaptor<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = xstd::basic_bit_array<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

} // namespace aligned

} // namespace xstd

#endif // XSTD_BITS_BIT_ARRAY_HPP
