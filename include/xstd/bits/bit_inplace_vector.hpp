//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_sequence_adaptor.hpp>            // bit_sequence_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // num_blocks_v
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <cstddef>                                       // size_t
#include <inplace_vector>                                // inplace_vector

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
using basic_bit_inplace_vector = bit_sequence_adaptor<std::inplace_vector<Block, bits::detail::num_blocks_v<Block, N>>>;

template<std::size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<std::size_t, N>;

} // namespace xstd

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_HPP
