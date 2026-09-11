//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STATIC_SET_HPP
#define XSTD_BITS_BIT_STATIC_SET_HPP

#include <xstd/bits/set_adaptor.hpp>               // set_adaptor
#include <xstd/bits/detail/block_array.hpp>        // block_array
#include <xstd/bits/ownership.hpp>                 // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/memory.hpp>                    // align_up
#include <cstddef>                                 // size_t
#include <limits>                                  // digits

namespace xstd {

// The static set: the qualifier marks the special case, the unmarked name going to the flagship. The basic name leaves the block open, the restricted one is the machine word. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block>
using basic_bit_static_set = set_adaptor<detail::bits::block_array<Block, N>, ownership::owns>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<N, std::size_t>;

namespace aligned {

template<std::size_t N, xstd::unsigned_integer Block>
using basic_bit_static_set = xstd::basic_bit_static_set<xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), Block>;

template<std::size_t N>
using bit_static_set = basic_bit_static_set<N, std::size_t>;

}       // namespace aligned
}       // namespace xstd

#endif  // XSTD_BITS_BIT_STATIC_SET_HPP
