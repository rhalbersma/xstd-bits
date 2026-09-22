//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_GRID_HPP
#define XSTD_BITS_GRID_HPP

#include <xstd/bits/ownership.hpp>                 // storage, window
#include <xstd/bits/tags.hpp>                      // container_tag, reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <span>                                    // dynamic_extent

namespace xstd {

// The storage a container tag names, answered where that storage is; a void allocator means the container's own.
template<container_tag C, xstd::unsigned_integer Block, std::size_t N = std::dynamic_extent, class Alloc = void>
struct bits_of;

template<container_tag C, xstd::unsigned_integer Block, std::size_t N = std::dynamic_extent, class Alloc = void>
using bits_t = bits_of<C, Block, N, Alloc>::type;

// The adaptor a reading tag names: declared here, defined by a partial specialization per reading.
template<reading_tag R, class Bits, storage Store = storage::owned, window W = window::all, class Derived = void>
class adaptor;

} // namespace xstd

#endif // XSTD_BITS_GRID_HPP
