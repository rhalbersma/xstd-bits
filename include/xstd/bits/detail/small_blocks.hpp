//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SMALL_BLOCKS_HPP
#define XSTD_BITS_DETAIL_SMALL_BLOCKS_HPP

// The small owners' storage, chosen for the whole program by XSTD_BITS_SMALL_VECTOR, which CMake sets on xstd::bits.

#ifdef XSTD_BITS_SMALL_VECTOR_ABSL

#include <xstd/bits/detail/bit_block_container.hpp> // move_assigns_without_throwing_v, swaps_without_throwing_v
#include <absl/container/inlined_vector.h>          // InlinedVector
#include <cstddef>                                  // size_t
#include <memory>                                   // allocator, allocator_traits

// The inline namespace the small owners are declared in, so that the storage is part of every mangled name.
#define XSTD_BITS_SMALL_STORAGE with_absl

namespace xstd::bits::detail {

template<class Block, std::size_t K, class Alloc>
using small_blocks = absl::InlinedVector<Block, K, Alloc>;

// absl::InlinedVector's own default.
template<class Block>
using small_allocator = std::allocator<Block>;

// absl::InlinedVector swaps allocators and adopts storage without its allocator, which only equal ones survive.
template<class Alloc>
concept small_blocks_allocator = std::allocator_traits<Alloc>::is_always_equal::value;

// Unsigned blocks under std::allocator: absl::InlinedVector swaps and move-assigns them by memcpy and nothing else.
template<class Block, std::size_t K>
inline constexpr bool swaps_without_throwing_v<absl::InlinedVector<Block, K, std::allocator<Block>>> = true;

template<class Block, std::size_t K>
inline constexpr bool move_assigns_without_throwing_v<absl::InlinedVector<Block, K, std::allocator<Block>>> = true;

} // namespace xstd::bits::detail

#else

#include <boost/container/new_allocator.hpp> // new_allocator
#include <boost/container/small_vector.hpp>  // small_vector
#include <cstddef>                           // size_t

#define XSTD_BITS_SMALL_STORAGE with_boost

namespace xstd::bits::detail {

template<class Block, std::size_t K, class Alloc>
using small_blocks = boost::container::small_vector<Block, K, Alloc>;

// boost::container::small_vector's own default.
template<class Block>
using small_allocator = boost::container::new_allocator<Block>;

// boost::container::small_vector honours every propagation trait, so any allocator holds its blocks.
template<class Alloc>
concept small_blocks_allocator = true;

} // namespace xstd::bits::detail

#endif

#endif // XSTD_BITS_DETAIL_SMALL_BLOCKS_HPP
