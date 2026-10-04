//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_BLOCKS_HPP

#include <xstd/bits/detail/range_const_reference.hpp> // range_const_reference_t
#include <xstd/bits/detail/static_block_capacity.hpp> // static_block_capacity
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <xstd/ints/limits.hpp>                       // numeric_limits
#include <array>                                      // array
#include <concepts>                                   // convertible_to, regular, same_as
#include <cstddef>                                    // size_t
#include <ranges>                                     // contiguous_range, end, range, range_reference_t, range_size_t, range_value_t, sized_range
#include <span>                                       // dynamic_extent, span
#include <type_traits>                                // remove_const_t

// What every container and view here presents a packed interface over: bits in contiguous unsigned blocks.
namespace xstd {

// One unsigned block, or a sized contiguous range of them that subscripts; const where a view only reads.
template<class Bits>
concept bit_blocks =
        xstd::unsigned_integer<std::remove_const_t<Bits>> or
        (std::ranges::sized_range<Bits> and std::ranges::contiguous_range<Bits> and
         xstd::unsigned_integer<std::remove_const_t<std::ranges::range_value_t<Bits>>> and
         requires (Bits& bits, std::ranges::range_size_t<Bits> n) { bits[n]; });

// Bit storage a container can own: a value compared by its blocks, and read-only through a const object.
template<class Bits>
concept owned_bit_blocks =
        bit_blocks<Bits> and std::regular<Bits> and
        (xstd::unsigned_integer<Bits> or
         requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n) {
                 { bits[n] } -> std::same_as<std::ranges::range_reference_t<Bits>>;
                 // P2278R4's alias: a storage whose const subscript yields a writable reference is refused.
                 { cbits[n] } -> std::same_as<bits::detail::range_const_reference_t<Bits>>;
         });

// Owned blocks whose count changes at run time: what an owner of a run-time width grows and shrinks.
template<class Bits>
concept resizable_bit_blocks =
        owned_bit_blocks<Bits> and std::ranges::range<Bits> and
        requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n, std::ranges::range_value_t<Bits> const* blocks) {
                bits.resize(n, *blocks);
                bits.push_back(*blocks);
                bits.insert(std::ranges::end(bits), blocks, blocks);
                bits.clear();
                { cbits.max_size() } -> std::convertible_to<std::ranges::range_size_t<Bits>>;
        };

// The width bit storage names by its type: every bit of a block or of a fixed number of blocks, else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_extent_v = std::dynamic_extent;

template<bit_blocks Bits>
        requires xstd::unsigned_integer<std::remove_const_t<Bits>>
inline constexpr std::size_t bit_blocks_extent_v<Bits> = static_cast<std::size_t>(xstd::numeric_limits<std::remove_const_t<Bits>>::digits);

template<xstd::unsigned_integer Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K>> = K * bit_blocks_extent_v<Block>;

template<xstd::unsigned_integer Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K> const> = bit_blocks_extent_v<std::array<Block, K>>;

template<class Block, std::size_t E>
        requires xstd::unsigned_integer<std::remove_const_t<Block>> and (E != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_extent_v<std::span<Block, E>> = E * bit_blocks_extent_v<Block>;

// The most bits an owner holds by its storage's type: a fixed width, else a constant capacity, else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_capacity_v = bit_blocks_extent_v<Bits>;

// The capacity in bits; boost::container::small_vector's static_capacity is its inline part and bounds nothing.
template<bit_blocks Bits>
        requires (bit_blocks_extent_v<Bits> == std::dynamic_extent) and resizable_bit_blocks<Bits> and (bits::detail::static_block_capacity<Bits>() != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_capacity_v<Bits> = bits::detail::static_block_capacity<Bits>() * bit_blocks_extent_v<std::ranges::range_value_t<Bits>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_BLOCKS_HPP
