//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STATIC_SET_HPP
#define XSTD_BITS_BIT_STATIC_SET_HPP

#include <xstd/bits/basic_bit_set.hpp>             // basic_bit_set
#include <xstd/bits/block_sequence.hpp>            // block_array
#include <xstd/bits/ownership.hpp>                 // ownership
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/memory.hpp>                    // align_up
#include <cstddef>                                 // size_t
#include <limits>                                  // digits

namespace xstd {

// The static set: the qualifier marks the special case, the unmarked name going to the flagship. [design.md#the-public-names]
template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
using bit_static_set = basic_bit_set<block_array<Block, N>, ownership::owns>;

namespace aligned {

template<std::size_t N, xstd::unsigned_integer Block = std::size_t>
using bit_static_set = xstd::bit_static_set<xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), Block>;

}       // namespace aligned
}       // namespace xstd

#endif  // XSTD_BITS_BIT_STATIC_SET_HPP
