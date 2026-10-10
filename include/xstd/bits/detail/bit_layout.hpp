//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_LAYOUT_HPP
#define XSTD_BITS_DETAIL_BIT_LAYOUT_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/owned_bit_blocks.hpp>           // owned_bit_blocks
#include <xstd/bits/detail/shift.hpp>                      // shl, shr
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <xstd/ints/limits.hpp>                            // numeric_limits
#include <algorithm>                                       // copy, min
#include <array>                                           // array
#include <bit>                                             // bit_cast, endian
#include <cstddef>                                         // size_t
#include <cstring>                                         // memcpy
#include <iterator>                                        // size
#include <limits>                                          // numeric_limits
#include <memory>                                          // addressof
#include <ranges>                                          // iota, range_value_t, size
#include <span>                                            // as_bytes, as_writable_bytes, dynamic_extent, span
#include <type_traits>                                     // is_bounded_array_v, is_class_v, is_trivially_copyable_v, remove_const_t

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
        (std::is_bounded_array_v<Bits> or owned_bit_blocks<Bits>) and
        xstd::bit_blocks_extent_v<Bits> >= N;

// A copy answers what the shifts do only when every bit of the object is a value bit: digits against sizeof.
template<class Block>
inline constexpr auto value_bits_fill_object =
        static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) == bits_per_byte * sizeof(Block);

// A block whose object is its value's bytes, least significant first: a built-in, little-endian, without padding.
template<class Block>
inline constexpr bool block_copies_as_bytes =
        std::endian::native == std::endian::little and (not std::is_class_v<Block>) and value_bits_fill_object<Block>;

template<class Bits>
[[nodiscard]] constexpr auto object_bytes(Bits const& b) noexcept
        -> std::array<unsigned char, sizeof(Bits)>
{
        return std::bit_cast<std::array<unsigned char, sizeof(Bits)>>(b);
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
inline constexpr auto bytes_per_block = byte_count<block_digits<Blocks>>;

// The bytes a range of blocks holds as positions, its value bits alone, at every block width.
template<class Blocks>
[[nodiscard]] constexpr auto value_bytes(Blocks const& blocks) noexcept
        -> std::size_t
{
        return std::ranges::size(blocks) * bytes_per_block<Blocks>;
}

// Byte j of a range of blocks: the positions [8j, 8j + 8), at every block width.
template<class Blocks>
[[nodiscard]] constexpr auto block_byte(Blocks const& blocks, std::size_t j) noexcept
        -> unsigned char
{
        auto const block = blocks[j / bytes_per_block<Blocks>];
        auto const shift = bits_per_byte * (j % bytes_per_block<Blocks>);
        return static_cast<unsigned char>(shr(block, shift));
}

// The write side, an or into blocks that start clear, so every byte is written once whatever the order.
template<class T, std::size_t E>
constexpr auto or_block_byte(std::span<T, E> blocks, std::size_t j, unsigned char byte) noexcept
        -> void
{
        constexpr auto per_block = bytes_per_block<std::span<T, E>>;
        // Through unsigned, as an unsigned char would promote to int and choose absl::uint128's signed constructor.
        auto const value = static_cast<T>(static_cast<unsigned>(byte));
        auto& block      = blocks[j / per_block];
        block            = static_cast<T>(block | shl(value, bits_per_byte * (j % per_block)));
}

template<class Src, class T, std::size_t E>
constexpr auto copy_bytes_by_shifts(Src const& src, std::span<T, E> dst, std::size_t bytes) noexcept
        -> void
{
        for (auto const j : std::views::iota(0UZ, bytes)) {
                or_block_byte(dst, j, block_byte(src, j));
        }
}

// Blocks to blocks at any two block widths, byte j to byte j, as far as the shorter reaches; the target starts clear.
template<class Src, class T, std::size_t E>
constexpr auto copy_bits(Src const& src, std::span<T, E> dst) noexcept
        -> void
{
        auto const bytes = std::ranges::min(value_bytes(src), value_bytes(dst));
        // Two alternatives rather than an early return, or MSVC's C4702 calls the shifts unreachable.
        if consteval {
                copy_bytes_by_shifts(src, dst, bytes);
        } else {
                if constexpr (block_copies_as_bytes<std::ranges::range_value_t<Src>> and block_copies_as_bytes<std::remove_const_t<T>>) {
                        // The object bytes in one copy, as memcpy, but over spans, where a null data() is no argument.
                        std::ranges::copy(std::as_bytes(std::span(src)).first(bytes), std::as_writable_bytes(dst).begin());
                } else {
                        copy_bytes_by_shifts(src, dst, bytes);
                }
        }
}

// A block as the range of one it is, and a range of blocks as itself, so that both copy as blocks.
template<class Bits>
[[nodiscard]] constexpr auto block_span(Bits& b) noexcept
{
        if constexpr (xstd::unsigned_integer<std::remove_const_t<Bits>>) {
                return std::span<Bits, 1>(std::addressof(b), 1UZ);
        } else {
                return std::span(b);
        }
}

template<std::size_t N, class Bits>
        requires bit_layout<Bits, N>
[[nodiscard]] constexpr auto bit_bytes(Bits const& b) noexcept
        -> std::array<unsigned char, byte_count<N>>
{
        auto bytes = std::array<unsigned char, byte_count<N>>();
        if constexpr (fixed_blocks_source<Bits, N>) {
                copy_bits(block_span(b), std::span(bytes));
        } else if constexpr (byte_count<N> > 0UZ) {
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
        return bytes;
}

template<class Bits, std::size_t N>
        requires bit_layout<Bits, N>
[[nodiscard]] constexpr auto bytes_bits(std::array<unsigned char, byte_count<N>> const& bytes) noexcept
        -> Bits
{
        if constexpr (fixed_blocks_source<Bits, N>) {
                // Value-initialised first, clearing the blocks above N, so set -> blocks -> set is the identity.
                auto blocks = Bits{};
                copy_bits(bytes, block_span(blocks));
                return blocks;
        } else if constexpr (byte_count<N> == 0UZ) {
                // No byte to read, and bit_cast of std::bitset<0> reads uninitialised: zero bytes are cast in.
                return std::bit_cast<Bits>(std::array<unsigned char, sizeof(Bits)>());
        } else {
                // Zero bytes past the width and a bit_cast, so nothing of Bits is constructed or called.
                auto object = std::array<unsigned char, sizeof(Bits)>();
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
