//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/grid.hpp>                            // bits_of
#include <xstd/bits/tags.hpp>                            // vector_container_tag
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <span>                                          // dynamic_extent
#include <memory>                                        // allocator
#include <vector>                                        // vector

namespace xstd::detail::bits {

// The second vehicle: a width on the heap, growing as a set of positions does.
template<xstd::unsigned_integer Block, class Allocator = std::allocator<Block>>
using contiguous_bit_vector = contiguous_bit_container<std::vector<Block, Allocator>>;

} // namespace xstd::detail::bits

namespace xstd {

// The extent is pinned, so a static one asked of an allocating container is a non-match rather than a silent drop.
template<xstd::unsigned_integer Block, class Alloc>
struct bits_of<vector_container_tag, Block, std::dynamic_extent, Alloc>
{
        using type = detail::bits::contiguous_bit_vector<Block, Alloc>;
};

// Named no allocator, so this tag supplies the one its own container defaults to rather than one the grid chose.
template<xstd::unsigned_integer Block>
struct bits_of<vector_container_tag, Block, std::dynamic_extent, void>
{
        using type = detail::bits::contiguous_bit_vector<Block>;
};

} // namespace xstd

#endif // XSTD_BITS_DETAIL_CONTIGUOUS_BIT_VECTOR_HPP
