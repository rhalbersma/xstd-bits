//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_INPLACE_VECTOR_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_inplace_vector

// The third vehicle: a run-time width under a compile-time capacity of N bits.
#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/detail/grid.hpp>                     // bits_of
#include <xstd/bits/detail/tags.hpp>                     // inplace_vector_container_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <cstddef>                                       // size_t
#include <inplace_vector>                                // inplace_vector

namespace xstd::detail::bits {

template<xstd::unsigned_integer Block, std::size_t N>
using contiguous_bit_inplace_vector = contiguous_bit_container<std::inplace_vector<Block, num_blocks_v<Block, N>>>;

} // namespace xstd::detail::bits

namespace xstd {

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct bits_of<inplace_vector_container_tag, Block, N, Alloc>
{
        using type = detail::bits::contiguous_bit_inplace_vector<Block, N>;
};

} // namespace xstd

#endif // __cpp_lib_inplace_vector

#endif // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_INPLACE_VECTOR_HPP
