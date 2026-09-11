//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_INPLACE_SET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

// The column comes and goes with its storage, and an alias withholds a name rather than a capability. [design.md#the-inplace-column]
#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/detail/block_inplace_vector.hpp> // block_inplace_vector
#include <xstd/bits/ownership.hpp>                   // ownership
#include <xstd/bits/set_adaptor.hpp>                 // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>   // unsigned_integer
#include <cstddef>                                   // size_t

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: inplace names where the storage lives. [design.md#the-public-names]
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_inplace_set = set_adaptor<detail::bits::block_inplace_vector<Block, N>, ownership::owns>;

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, N>;

}       // namespace xstd

#endif  // __cpp_lib_inplace_vector

#endif  // XSTD_BITS_BIT_INPLACE_SET_HPP
