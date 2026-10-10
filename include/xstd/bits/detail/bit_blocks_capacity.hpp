//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_BLOCKS_CAPACITY_HPP
#define XSTD_BITS_DETAIL_BIT_BLOCKS_CAPACITY_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/resizable_bit_blocks.hpp>       // resizable_bit_blocks
#include <cstddef>                                         // size_t
#include <ranges>                                          // range_value_t
#include <span>                                            // dynamic_extent
#include <type_traits>                                     // integral_constant, is_pointer_v

namespace xstd::bits::detail {

// In blocks, the capacity the type answers without an object, else dynamic_extent.
template<class Bits>
consteval auto static_block_capacity() noexcept
        -> std::size_t
{
        // A capacity() usable as a constant, as std::inplace_vector's is.
        if constexpr (requires { typename std::integral_constant<std::size_t, Bits::capacity()>; }) {
                return Bits::capacity();
        } else if constexpr (requires { requires std::is_pointer_v<decltype(&Bits::capacity)>; typename std::integral_constant<std::size_t, Bits::static_capacity>; }) {
                // A static run-time capacity(), as boost::container::static_vector's, names static_capacity too.
                return Bits::static_capacity;
        } else {
                return std::dynamic_extent;
        }
}

// The most bits an owner holds by its storage's type: a fixed width, else a constant capacity, else dynamic_extent.
template<xstd::bit_blocks Bits>
inline constexpr std::size_t bit_blocks_capacity_v = xstd::bit_blocks_extent_v<Bits>;

// The capacity in bits; boost::container::small_vector's static_capacity is its inline part and bounds nothing.
template<xstd::bit_blocks Bits>
        requires (xstd::bit_blocks_extent_v<Bits> == std::dynamic_extent) and resizable_bit_blocks<Bits> and (static_block_capacity<Bits>() != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_capacity_v<Bits> = static_block_capacity<Bits>() * xstd::bit_blocks_extent_v<std::ranges::range_value_t<Bits>>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_BLOCKS_CAPACITY_HPP
