//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_INPLACE_BITSET_HPP
#define XSTD_BITS_INPLACE_BITSET_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/basic_bits.hpp>                           // basic_bits
#include <xstd/bits/bitset_adaptor.hpp>                       // IWYU pragma: keep; the adaptor bitset_reading_tag names
#include <xstd/bits/detail/contiguous_bit_inplace_vector.hpp> // IWYU pragma: keep; the storage inplace_vector_container_tag names
#include <xstd/bits/tags.hpp>                                 // bitset_reading_tag, inplace_vector_container_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <cstddef>                                            // size_t

namespace xstd {

// A resizable bitset that never allocates; no bit_ prefix, bitset already carrying the word.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_inplace_bitset = basic_bits<bitset_reading_tag, inplace_vector_container_tag, Block, N>;

template<std::size_t N>
using inplace_bitset = basic_inplace_bitset<std::size_t, N>;

} // namespace xstd

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_INPLACE_BITSET_HPP
