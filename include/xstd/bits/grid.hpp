//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_GRID_HPP
#define XSTD_BITS_GRID_HPP

#include <xstd/bits/tags.hpp>                      // container_tag, reading_tag
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <memory>                                  // allocator
#include <span>                                    // dynamic_extent

namespace xstd {

// The storage a container tag names, each tag answering in the header that defines the storage it names.
template<container_tag C, xstd::unsigned_integer Block, std::size_t N = std::dynamic_extent, class Alloc = std::allocator<Block>>
struct bits_of;

template<container_tag C, xstd::unsigned_integer Block, std::size_t N = std::dynamic_extent, class Alloc = std::allocator<Block>>
using bits_t = bits_of<C, Block, N, Alloc>::type;

// The adaptor a reading tag names, each tag answering in the header that defines the adaptor it names.
template<reading_tag R, class Bits, class Derived>
struct adaptor_of;

template<reading_tag R, class Bits, class Derived>
using adaptor_t = adaptor_of<R, Bits, Derived>::type;

} // namespace xstd

#endif // XSTD_BITS_GRID_HPP
