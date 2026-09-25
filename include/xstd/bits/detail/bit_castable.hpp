//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
#define XSTD_BITS_DETAIL_BIT_CASTABLE_HPP

#include <xstd/bits/detail/contiguous_block_range.hpp> // contiguous_block_range
#include <xstd/ints/concepts/unsigned_integer.hpp>     // unsigned_integer
#include <xstd/ints/limits.hpp>                        // numeric_limits
#include <array>                                       // array
#include <bit>                                         // bit_cast, endian
#include <concepts>                                    // convertible_to, default_initializable
#include <cstddef>                                     // byte, size_t, to_integer
#include <cstring>                                     // memcpy
#include <limits>                                      // numeric_limits
#include <memory>                                      // addressof
#include <ranges>                                      // contiguous_range, data, iota, range_value_t
#include <type_traits>                                 // bool_constant, is_trivially_copyable_v

namespace xstd::bits::detail {

inline constexpr auto bits_per_byte = static_cast<std::size_t>(std::numeric_limits<unsigned char>::digits);
inline constexpr auto bits_per_word = static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);

// The bytes a width needs: byte j holds the positions [8j, 8j + 8) least significant bit first, at every block width.
template<std::size_t N>
inline constexpr auto byte_count = (N + bits_per_byte - 1UZ) / bits_per_byte;

// An unsigned integer states its own layout: bit n of the value is 2^n, by the language, so there is nothing to probe.
template<class B, std::size_t N>
concept integer_source =
        xstd::unsigned_integer<B> and
        N <= static_cast<std::size_t>(xstd::numeric_limits<B>::digits);

// The same family over a sequence: block j holds [j*digits, (j+1)*digits), and a scalar is the length-one case.
template<class B>
inline constexpr auto block_digits = static_cast<std::size_t>(
        xstd::numeric_limits<std::ranges::range_value_t<B>>::digits
);

template<class B>
concept block_size_is_constant = requires {
        typename std::bool_constant<(B().size(), true)>;
};

template<class B, std::size_t N>
concept block_range_source =
        // Contiguous first: contiguous_block_range asks constructible_from, which re-enters this very constraint.
        std::ranges::contiguous_range<B> and
        contiguous_block_range<B> and
        std::default_initializable<B> and
        block_size_is_constant<B> and
        B().size() * block_digits<B> >= N;

// A copy answers what the shifts do only when every bit of the object is a value bit: digits against sizeof.
template<class B>
inline constexpr auto value_bits_fill_object =
        static_cast<std::size_t>(xstd::numeric_limits<B>::digits) == bits_per_byte * sizeof(B);

template<class B>
inline constexpr auto blocks_copy_as_bytes =
        std::endian::native == std::endian::little and value_bits_fill_object<std::ranges::range_value_t<B>>;

template<class B>
[[nodiscard]] constexpr auto object_bytes(B const& b) noexcept
        -> std::array<std::byte, sizeof(B)>
{
        return std::bit_cast<std::array<std::byte, sizeof(B)>>(b);
}

// Whether bit_cast of a value-initialised B is a constant expression, asked so it answers false rather than erroring.
template<class B>
concept bit_cast_is_constant = requires {
        typename std::bool_constant<(object_bytes(B()), true)>;
};

// What the probe below has to be able to ask of a candidate: build an empty one, light one position, count them.
template<class B>
concept probeable_bits =
        std::default_initializable<B> and
        requires (B& b, B const& c, std::size_t n) {
                b.set(n);
                { c.count() } -> std::convertible_to<std::size_t>;
                { c.size() } -> std::convertible_to<std::size_t>;
        };

// One position lit and counted, asked only for whether it is a constant expression.
template<class B>
[[nodiscard]] constexpr auto probe_once()
        -> bool
{
        auto b = B();
        b.set(0UZ);
        return static_cast<std::size_t>(b.count()) == 1UZ;
}

template<class B>
concept probe_is_constant = requires {
        typename std::bool_constant<(probe_once<B>(), true)>;
};

// All clear, then one position at a time: with count() == 1, no other byte can hold anything.
template<class B, std::size_t N>
[[nodiscard]] constexpr auto bit_layout_holds()
        -> bool
{
        for (auto const byte : object_bytes(B())) {
                if (byte != std::byte{}) {
                        return false;
                }
        }
        for (auto const i : {0UZ, 7UZ, bits_per_byte, bits_per_word, N - 1UZ}) {
                if (i >= N) {
                        continue;
                }
                auto b = B();
                b.set(i);
                if (static_cast<std::size_t>(b.count()) != 1UZ) {
                        return false;
                }
                if (object_bytes(b)[i / bits_per_byte] != static_cast<std::byte>(1U << (i % bits_per_byte))) {
                        return false;
                }
        }
        return true;
}

// The second family: a field of bits whose layout is proved rather than believed, cheap structural questions first.
template<class B, std::size_t N>
concept container_source =
        probeable_bits<B> and
        std::is_trivially_copyable_v<B> and
        // Its width, not merely one that fits, and asked before the probe so it never reaches a missing position.
        B().size() == N and
        sizeof(B) * bits_per_byte >= N and
        sizeof(B) * bits_per_byte < N + bits_per_word and
        // With no byte to exchange there is nothing to prove, and bit_cast of std::bitset<0> reads uninitialised.
        (byte_count<N> == 0UZ or (bit_cast_is_constant<B> and probe_is_constant<B> and bit_layout_holds<B, N>()));

template<class B, std::size_t N>
concept bit_castable = integer_source<B, N> or block_range_source<B, N> or container_source<B, N>;

// The shifts, in one place: they say where a position goes rather than assuming a byte order.
template<std::size_t N, class B, std::size_t E>
constexpr auto block_bytes_by_shifts(B const& b, std::array<std::byte, E>& bytes) noexcept
        -> void
{
        constexpr auto bytes_per_block = block_digits<B> / bits_per_byte;
        for (auto const j : std::views::iota(0UZ, bytes.size())) {
                auto const block = b[j / bytes_per_block];
                auto const shift = bits_per_byte * (j % bytes_per_block);
                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(block >> shift));
        }
}

template<std::size_t N, class B, std::size_t E>
constexpr auto bytes_blocks_by_shifts(std::array<std::byte, E> const& bytes, B& blocks) noexcept
        -> void
{
        using block_type = std::ranges::range_value_t<B>;
        constexpr auto bytes_per_block = block_digits<B> / bits_per_byte;
        for (auto const j : std::views::iota(0UZ, bytes.size())) {
                auto const byte = static_cast<block_type>(std::to_integer<unsigned char>(bytes[j]));
                auto const shift = bits_per_byte * (j % bytes_per_block);
                auto& block = blocks[j / bytes_per_block];
                block = static_cast<block_type>(block | static_cast<block_type>(byte << shift));
        }
}

template<std::size_t N, class B>
        requires bit_castable<B, N>
[[nodiscard]] constexpr auto bit_bytes(B const& b) noexcept
        -> std::array<std::byte, byte_count<N>>
{
        auto bytes = std::array<std::byte, byte_count<N>>();
        if constexpr (byte_count<N> > 0UZ) {
                if constexpr (integer_source<B, N>) {
                        // No copy: memcpy timed at 0.31ns either way, so the branch buys nothing.
                        for (auto const j : std::views::iota(0UZ, bytes.size())) {
                                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(b >> (bits_per_byte * j)));
                        }
                } else if constexpr (block_range_source<B, N>) {
                        // Two alternatives rather than an early return, or MSVC's C4702 calls the shifts unreachable.
                        if consteval {
                                block_bytes_by_shifts<N>(b, bytes);
                        } else {
                                if constexpr (blocks_copy_as_bytes<B>) {
                                        std::memcpy(bytes.data(), std::ranges::data(b), bytes.size());
                                } else {
                                        block_bytes_by_shifts<N>(b, bytes);
                                }
                        }
                } else {
                        // Trivially copyable by container_source, so the object representation reads straight out.
                        if consteval {
                                auto const object = object_bytes(b);
                                for (auto const j : std::views::iota(0UZ, bytes.size())) {
                                        bytes[j] = object[j];
                                }
                        } else {
                                std::memcpy(bytes.data(), std::addressof(b), bytes.size());
                        }
                }
        }
        return bytes;
}

template<class B, std::size_t N>
        requires bit_castable<B, N>
[[nodiscard]] constexpr auto bytes_bits(std::array<std::byte, byte_count<N>> const& bytes) noexcept
        -> B
{
        if constexpr (byte_count<N> == 0UZ) {
                return B();
        } else if constexpr (integer_source<B, N>) {
                // The shifts alone, for the reason bit_bytes gives: a copy measured the same and said less.
                auto value = B();
                for (auto const j : std::views::iota(0UZ, bytes.size())) {
                        auto const byte = static_cast<B>(std::to_integer<unsigned char>(bytes[j]));
                        value = static_cast<B>(value | static_cast<B>(byte << (bits_per_byte * j)));
                }
                return value;
        } else if constexpr (block_range_source<B, N>) {
                // Value-initialised first, clearing the blocks above N, so set -> blocks -> set is the identity.
                auto blocks = B();
                if consteval {
                        bytes_blocks_by_shifts<N>(bytes, blocks);
                } else {
                        if constexpr (blocks_copy_as_bytes<B>) {
                                std::memcpy(std::ranges::data(blocks), bytes.data(), bytes.size());
                        } else {
                                bytes_blocks_by_shifts<N>(bytes, blocks);
                        }
                }
                return blocks;
        } else {
                auto object = std::array<std::byte, sizeof(B)>();
                if consteval {
                        for (auto const j : std::views::iota(0UZ, bytes.size())) {
                                object[j] = bytes[j];
                        }
                } else {
                        std::memcpy(object.data(), bytes.data(), bytes.size());
                }
                return std::bit_cast<B>(object);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
