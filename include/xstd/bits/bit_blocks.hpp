//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_BLOCKS_HPP

#include <xstd/bits/detail/range_const_reference.hpp> // range_const_reference_t
#include <xstd/bits/detail/rebind.hpp>                // rebind_block_t, rebind_t, rebind_width_v, rebindable
#include <xstd/bits/detail/static_block_capacity.hpp> // static_block_capacity
#include <xstd/ints/concepts/integer.hpp>             // integer
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <xstd/ints/limits.hpp>                       // numeric_limits
#include <xstd/ints/memory.hpp>                       // align_up
#include <array>                                      // array
#include <concepts>                                   // convertible_to, integral, regular, same_as
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint16_t, uint32_t, uint64_t, uint8_t, uint_fast16_t, uint_fast32_t, uint_fast64_t, uint_fast8_t
#include <ranges>                                     // contiguous_range, end, range, range_reference_t, range_size_t, range_value_t, sized_range
#include <span>                                       // dynamic_extent, span
#include <type_traits>                                // conditional_t, is_enum_v, make_unsigned_t, remove_cv_t, type_identity, underlying_type, underlying_type_t

// What every container and view here presents a packed interface over: bits in contiguous unsigned blocks.
namespace xstd {

// One unsigned block, const where a view only reads.
template<class Block>
concept bit_block = xstd::unsigned_integer<Block>;

// A sized contiguous range of blocks that subscripts.
template<class Blocks>
concept bit_block_range =
        std::ranges::sized_range<Blocks> and std::ranges::contiguous_range<Blocks> and
        bit_block<std::ranges::range_value_t<Blocks>> and
        requires (Blocks& blocks, std::ranges::range_size_t<Blocks> n) { blocks[n]; };

// One block, or a range of them: what every container and view here holds its bits in.
template<class Bits>
concept bit_blocks = bit_block<Bits> or bit_block_range<Bits>;

// Bit storage a container can own: a value compared by its blocks, and read-only through a const object.
template<class Bits>
concept owned_bit_blocks =
        bit_blocks<Bits> and std::regular<Bits> and
        (bit_block<Bits> or
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
        requires bit_block<Bits>
inline constexpr std::size_t bit_blocks_extent_v<Bits> = static_cast<std::size_t>(xstd::numeric_limits<Bits>::digits);

template<bit_block Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K>> = bit_blocks_extent_v<Block> * K;

template<bit_block Block, std::size_t K>
inline constexpr std::size_t bit_blocks_extent_v<std::array<Block, K> const> = bit_blocks_extent_v<std::array<Block, K>>;

template<bit_block Block, std::size_t E>
        requires (E != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_extent_v<std::span<Block, E>> = bit_blocks_extent_v<Block> * E;

// A built-in array names its bound as std::array does, over blocks of any constness.
template<class Block, std::size_t K>
        requires bit_block<Block>
inline constexpr std::size_t bit_blocks_extent_v<Block[K]> = bit_blocks_extent_v<std::remove_cv_t<Block>> * K; // NOLINT(modernize-avoid-c-arrays): a built-in array is what it names.

// The most bits an owner holds by its storage's type: a fixed width, else a constant capacity, else dynamic_extent.
template<bit_blocks Bits>
inline constexpr std::size_t bit_blocks_capacity_v = bit_blocks_extent_v<Bits>;

// The capacity in bits; boost::container::small_vector's static_capacity is its inline part and bounds nothing.
template<bit_blocks Bits>
        requires (bit_blocks_extent_v<Bits> == std::dynamic_extent) and resizable_bit_blocks<Bits> and (bits::detail::static_block_capacity<Bits>() != std::dynamic_extent)
inline constexpr std::size_t bit_blocks_capacity_v<Bits> = bits::detail::static_block_capacity<Bits>() * bit_blocks_extent_v<std::ranges::range_value_t<Bits>>;

// The narrowest fixed-width block holding N bits in one, else std::uint64_t, of which N bits then take several.
template<std::size_t N>
using least_block_t = std::conditional_t<
        (N <= bit_blocks_extent_v<std::uint8_t>), std::uint8_t,
        std::conditional_t<
                (N <= bit_blocks_extent_v<std::uint16_t>), std::uint16_t,
                std::conditional_t<(N <= bit_blocks_extent_v<std::uint32_t>), std::uint32_t, std::uint64_t>>>;

// The fastest fixed-width block of at least N bits in one, else std::uint_fast64_t, of which N bits then take several.
template<std::size_t N>
using fast_block_t = std::conditional_t<
        (N <= bit_blocks_extent_v<std::uint8_t>), std::uint_fast8_t,
        std::conditional_t<
                (N <= bit_blocks_extent_v<std::uint16_t>), std::uint_fast16_t,
                std::conditional_t<(N <= bit_blocks_extent_v<std::uint32_t>), std::uint_fast32_t, std::uint_fast64_t>>>;

// An enumeration's underlying type or an integer type made unsigned, the block a field or ABI of it already uses.
template<class Key>
        requires (std::is_enum_v<Key> and (not std::same_as<std::underlying_type_t<Key>, bool>) and bit_block<std::make_unsigned_t<std::underlying_type_t<Key>>>) or (std::integral<Key> and xstd::integer<Key>)
using underlying_block_t = std::make_unsigned_t<typename std::conditional_t<std::is_enum_v<Key>, std::underlying_type<Key>, std::type_identity<Key>>::type>;

// The same bit container in the smallest block that holds its N bits, as uint_least8_t is the smallest of at least 8.
template<class Bits>
        requires bits::detail::rebindable<Bits>
using bit_least = bits::detail::rebind_t<Bits, least_block_t<bits::detail::rebind_width_v<Bits>>, bits::detail::rebind_width_v<Bits>>;

// The same bit container in the fastest block of at least its N bits, as uint_fast8_t is the fastest of at least 8.
template<class Bits>
        requires bits::detail::rebindable<Bits>
using bit_fast = bits::detail::rebind_t<Bits, fast_block_t<bits::detail::rebind_width_v<Bits>>, bits::detail::rebind_width_v<Bits>>;

// The same bit container with N rounded up to a whole number of its blocks, so that no block carries an unused tail.
template<class Bits>
        requires bits::detail::rebindable<Bits>
using bit_align = bits::detail::rebind_t<Bits, bits::detail::rebind_block_t<Bits>, xstd::align_up(bits::detail::rebind_width_v<Bits>, bit_blocks_extent_v<bits::detail::rebind_block_t<Bits>>)>;

// The same bit set in its key's underlying block, enumeration or integer, as an existing field or ABI stores it.
template<class Bits>
        requires bits::detail::rebindable<Bits> and requires { typename underlying_block_t<typename Bits::key_type>; }
using bit_underlying = bits::detail::rebind_t<Bits, underlying_block_t<typename Bits::key_type>, bits::detail::rebind_width_v<Bits>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_BLOCKS_HPP
