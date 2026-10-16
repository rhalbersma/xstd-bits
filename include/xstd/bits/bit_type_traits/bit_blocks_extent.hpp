//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp> // bit_blocks
#include <xstd/bits/detail/blocks_extent.hpp>    // blocks_extent_v
#include <cstddef>                               // size_t
#include <type_traits>                           // remove_cv_t

namespace xstd {

// The width storage names by its type, cv aside: a block's bits or a fixed count of blocks', else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_extent_v = bits::detail::blocks_extent_v<std::remove_cv_t<Bits>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_EXTENT_HPP
