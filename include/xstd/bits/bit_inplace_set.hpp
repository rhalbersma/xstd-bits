//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_INPLACE_SET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/basic_bits.hpp>                           // basic_bits, inplace_vector_container_tag, set_reading_tag
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // contiguous_bit_inplace_vector
#include <xstd/bits/set_adaptor.hpp>                          // set_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: inplace names where the storage lives.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_inplace_set = basic_bits<set_reading_tag, inplace_vector_container_tag, Block, N>;

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, N>;

} // namespace xstd

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_INPLACE_SET_HPP
