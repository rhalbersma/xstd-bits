//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/ownership.hpp>                   // ownership
#include <xstd/bits/sequence_adaptor.hpp>            // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <xstd/ints/memory.hpp>                      // align_up
#include <cstddef>                                   // size_t
#include <limits>                                    // digits

namespace xstd {

// The packed std::array<bool, N>, named after the container it packs. [design.md#the-public-names]
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = sequence_adaptor<detail::bits::contiguous_bit_array<Block, N>, ownership::owns, false>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_array = xstd::basic_bit_array<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

}       // namespace aligned
}       // namespace xstd

#endif  // XSTD_BITS_BIT_ARRAY_HPP
