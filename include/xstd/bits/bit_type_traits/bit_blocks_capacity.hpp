//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_CAPACITY_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_CAPACITY_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_concepts/resizable_bit_blocks.hpp> // resizable_bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/static_block_capacity.hpp>      // static_block_capacity
#include <cstddef>                                         // size_t
#include <ranges>                                          // range_value_t
#include <span>                                            // dynamic_extent

namespace xstd {

// The most bits an owner holds by its storage's type: a fixed width, else a constant capacity, else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_capacity_v = bit_blocks_extent_v<Bits>;

// The capacity in bits; boost::container::small_vector's static_capacity is its inline part and bounds nothing.
template<bit_blocks Bits>
        requires (bit_blocks_extent_v<Bits> == std::dynamic_extent) and resizable_bit_blocks<Bits> and (bits::detail::static_block_capacity<Bits>() != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_capacity_v<Bits> = bits::detail::static_block_capacity<Bits>() * bit_blocks_extent_v<std::ranges::range_value_t<Bits>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_BLOCKS_CAPACITY_HPP
