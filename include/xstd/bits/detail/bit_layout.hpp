//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_LAYOUT_HPP
#define XSTD_BITS_DETAIL_BIT_LAYOUT_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp>            // bit_block
#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_concepts/owned_bit_blocks.hpp>     // owned_bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/ints/limits.hpp>                            // numeric_limits
#include <array>                                           // array
#include <bit>                                             // bit_cast, endian
#include <cstddef>                                         // byte, size_t, to_integer
#include <cstring>                                         // memcpy
#include <iterator>                                        // size
#include <limits>                                          // numeric_limits
#include <memory>                                          // addressof
#include <ranges>                                          // data, iota, range_value_t
#include <span>                                            // dynamic_extent, span
#include <type_traits>                                     // is_bounded_array_v, is_trivially_copyable_v

namespace xstd::bits::detail {

inline constexpr auto bits_per_byte = static_cast<std::size_t>(std::numeric_limits<unsigned char>::digits);

// The bytes a width needs: byte j holds the positions [8j, 8j + 8) least significant bit first, at every block width.
template<std::size_t N>
inline constexpr auto byte_count = (N + bits_per_byte - 1UZ) / bits_per_byte;

// Bit blocks whose type names their width: a block, or a fixed number of them.
template<class Bits>
concept fixed_bit_blocks = xstd::bit_blocks<Bits> and xstd::bit_blocks_extent_v<Bits> != std::dynamic_extent;

// The digits of each block in a range of them: block j holds the positions [j*digits, (j+1)*digits).
template<class Blocks>
inline constexpr auto block_digits = static_cast<std::size_t>(
        xstd::numeric_limits<std::ranges::range_value_t<Blocks>>::digits
);

// Blocks that state their own layout, at a width that holds N: bit n of a block is 2^n, by the language.
template<class Bits, std::size_t N>
concept fixed_blocks_source =
        // Fixed first: owned_bit_blocks asks constructible_from, which can re-enter this very constraint.
        fixed_bit_blocks<Bits> and
        // A built-in array is read and never owned.
        (std::is_bounded_array_v<Bits> or xstd::owned_bit_blocks<Bits>) and
        xstd::bit_blocks_extent_v<Bits> >= N;

// A copy answers what the shifts do only when every bit of the object is a value bit: digits against sizeof.
template<class Block>
inline constexpr auto value_bits_fill_object =
        static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) == bits_per_byte * sizeof(Block);

template<class Blocks>
inline constexpr auto blocks_copy_as_bytes =
        std::endian::native == std::endian::little and value_bits_fill_object<std::ranges::range_value_t<Blocks>>;

template<class Bits>
[[nodiscard]] constexpr auto object_bytes(Bits const& b) noexcept
        -> std::array<std::byte, sizeof(Bits)>
{
        return std::bit_cast<std::array<std::byte, sizeof(Bits)>>(b);
}

// The second family: a foreign field of bits, read under std::bit_cast's rules wherever its object has room for N bits.
template<class Bits, std::size_t N>
concept container_source =
        std::is_trivially_copyable_v<Bits> and
        std::endian::native == std::endian::little and
        N <= sizeof(Bits) * bits_per_byte;

template<class Bits, std::size_t N>
concept bit_layout = fixed_blocks_source<Bits, N> or container_source<Bits, N>;

// The shifts, in one place: they say where a position goes rather than assuming a byte order.
template<class Blocks>
inline constexpr auto bytes_per_block = block_digits<Blocks> / bits_per_byte;

// Byte j of a range of blocks: the positions [8j, 8j + 8), at every block width.
template<class Blocks>
[[nodiscard]] constexpr auto block_byte(Blocks const& blocks, std::size_t j) noexcept
        -> std::byte
{
        auto const block = blocks[j / bytes_per_block<Blocks>];
        auto const shift = bits_per_byte * (j % bytes_per_block<Blocks>);
        return static_cast<std::byte>(static_cast<unsigned char>(block >> shift));
}

// The write side, an or into blocks that start clear, so every byte is written once whatever the order.
template<class T, std::size_t E>
constexpr auto or_block_byte(std::span<T, E> blocks, std::size_t j, std::byte byte) noexcept
        -> void
{
        constexpr auto per_block = bytes_per_block<std::span<T, E>>;
        auto const value         = static_cast<T>(std::to_integer<unsigned char>(byte));
        auto& block              = blocks[j / per_block];
        block                    = static_cast<T>(block | static_cast<T>(value << (bits_per_byte * (j % per_block))));
}

template<class Blocks, std::size_t E>
constexpr auto block_bytes_by_shifts(Blocks const& b, std::array<std::byte, E>& bytes) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                bytes[j] = block_byte(b, j);
        }
}

template<class Blocks, std::size_t E>
constexpr auto bytes_blocks_by_shifts(std::array<std::byte, E> const& bytes, Blocks& blocks) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                or_block_byte(std::span(blocks), j, bytes[j]);
        }
}

template<std::size_t N, class Bits>
        requires bit_layout<Bits, N>
[[nodiscard]] constexpr auto bit_bytes(Bits const& b) noexcept
        -> std::array<std::byte, byte_count<N>>
{
        auto bytes = std::array<std::byte, byte_count<N>>();
        if constexpr (byte_count<N> > 0UZ) {
                if constexpr (xstd::bit_block<Bits>) {
                        // No copy: memcpy timed at 0.31ns either way, so the branch buys nothing.
                        for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(b >> (bits_per_byte * j)));
                        }
                } else if constexpr (fixed_blocks_source<Bits, N>) {
                        // Two alternatives rather than an early return, or MSVC's C4702 calls the shifts unreachable.
                        if consteval {
                                block_bytes_by_shifts(b, bytes);
                        } else {
                                if constexpr (blocks_copy_as_bytes<Bits>) {
                                        std::memcpy(bytes.data(), std::ranges::data(b), std::size(bytes));
                                } else {
                                        block_bytes_by_shifts(b, bytes);
                                }
                        }
                } else {
                        // Trivially copyable by container_source, so the object representation reads straight out.
                        if consteval {
                                auto const object = object_bytes(b);
                                for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                                        bytes[j] = object[j];
                                }
                        } else {
                                std::memcpy(bytes.data(), std::addressof(b), std::size(bytes));
                        }
                }
        }
        return bytes;
}

template<class Bits, std::size_t N>
        requires bit_layout<Bits, N>
[[nodiscard]] constexpr auto bytes_bits(std::array<std::byte, byte_count<N>> const& bytes) noexcept
        -> Bits
{
        if constexpr (byte_count<N> == 0UZ) {
                if constexpr (fixed_blocks_source<Bits, N>) {
                        return Bits{};
                } else {
                        // No byte to read, and bit_cast of std::bitset<0> reads uninitialised: zero bytes are cast in.
                        return std::bit_cast<Bits>(std::array<std::byte, sizeof(Bits)>());
                }
        } else if constexpr (xstd::bit_block<Bits>) {
                // The shifts alone, for the reason bit_bytes gives: a copy measured the same and said less.
                auto value = Bits{};
                for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                        auto const byte = static_cast<Bits>(std::to_integer<unsigned char>(bytes[j]));
                        value           = static_cast<Bits>(value | static_cast<Bits>(byte << (bits_per_byte * j)));
                }
                return value;
        } else if constexpr (fixed_blocks_source<Bits, N>) {
                // Value-initialised first, clearing the blocks above N, so set -> blocks -> set is the identity.
                auto blocks = Bits{};
                if consteval {
                        bytes_blocks_by_shifts(bytes, blocks);
                } else {
                        if constexpr (blocks_copy_as_bytes<Bits>) {
                                std::memcpy(std::ranges::data(blocks), bytes.data(), std::size(bytes));
                        } else {
                                bytes_blocks_by_shifts(bytes, blocks);
                        }
                }
                return blocks;
        } else {
                // Zero bytes past the width and a bit_cast, so nothing of Bits is constructed or called.
                auto object = std::array<std::byte, sizeof(Bits)>();
                if consteval {
                        for (auto const j : std::views::iota(0UZ, std::size(bytes))) {
                                object[j] = bytes[j];
                        }
                } else {
                        std::memcpy(object.data(), bytes.data(), std::size(bytes));
                }
                return std::bit_cast<Bits>(object);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_LAYOUT_HPP
