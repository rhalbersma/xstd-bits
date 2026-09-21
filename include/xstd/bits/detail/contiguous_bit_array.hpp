//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_ARRAY_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_ARRAY_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/grid.hpp>                            // bits_of
#include <xstd/bits/tags.hpp>                            // array_container_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <array>                                         // array
#include <cstddef>                                       // size_t

namespace xstd::detail::bits {

// The first vehicle: a width in the type, over storage that goes wherever the object does.
template<xstd::unsigned_integer Block, std::size_t N>
using contiguous_bit_array = contiguous_bit_container<std::array<Block, num_blocks_v<Block, N>>, N>;

} // namespace xstd::detail::bits

namespace xstd {

template<xstd::unsigned_integer Block, std::size_t N, class Alloc>
struct bits_of<array_container_tag, Block, N, Alloc>
{
        using type = detail::bits::contiguous_bit_array<Block, N>;
};

} // namespace xstd

#endif // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_ARRAY_HPP
