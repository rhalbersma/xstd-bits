//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
#define XSTD_BITS_DETAIL_BIT_CASTABLE_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <bit>                                     // bit_cast
#include <concepts>                                // convertible_to, default_initializable
#include <cstddef>                                 // byte, size_t, to_integer
#include <limits>                                  // numeric_limits
#include <type_traits>                             // bool_constant, is_trivially_copyable_v

namespace xstd::detail::bits {

inline constexpr auto bits_per_byte = static_cast<std::size_t>(std::numeric_limits<unsigned char>::digits);
inline constexpr auto bits_per_word = static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);

// The bytes a width needs, which is the currency two fields of bits are exchanged in: byte j holds the positions
// [8j, 8j + 8) least significant bit first, and that is true at every block width, which is what makes a byte the
// common ground where a word is not.
template<std::size_t N>
inline constexpr auto byte_count = (N + bits_per_byte - 1UZ) / bits_per_byte;

// A source of N bits comes in two families, and only one of them has anything to prove.
//
// An UNSIGNED INTEGER is its own layout: bit n of the value is 2^n, said by the language rather than by any
// implementation, so there is nothing here to assume and nothing to probe. That is the family unsigned long long
// belongs to, which is why to_ullong and the constructor taking one need no case of their own below -- they are
// this case.
template<class B, std::size_t N>
concept integer_source =
        xstd::unsigned_integer<B> and
        N <= static_cast<std::size_t>(xstd::numeric_limits<B>::digits)
;

template<class B>
[[nodiscard]] constexpr auto object_bytes(B const& b) noexcept
        -> std::array<std::byte, sizeof(B)>
{
        return std::bit_cast<std::array<std::byte, sizeof(B)>>(b);
}

// Whether bit_cast of a value-initialised B is a CONSTANT EXPRESSION, asked so that it answers false instead of
// erroring. It is what rules out a pointer member, a reference member and a union, which are the shapes bit_cast
// refuses to be constexpr for -- and it replaces has_unique_object_representations_v, which cannot be used here:
// GCC 13 through 16 answer false for ANY class with an empty non-static data member, even one [[no_unique_address]]
// costs nothing, where clang answers true. Every container this library defines has such a member, so that trait
// would make this concept false on one compiler and true on the other. Measured on both, at identical layout.
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
                { c.size()  } -> std::convertible_to<std::size_t>;
        }
;

// FIVE positions, and the count is the budget rather than the taste. Each probe materialises the whole byte array
// through bit_cast, so it costs O(sizeof(B)) however few bytes it then reads, and clang's default
// -fconstexpr-steps admits six of them at a width of 2^20 and refuses seven; GCC's limit is higher. Five leaves
// margin and puts the ceiling at 2^20 -- a 128 KiB object -- past which a caller raises -fconstexpr-steps.
//
// count() == 1 is what makes each probe cheap AND total: with no padding reachable, one position set and the
// expected byte holding exactly its bit, no OTHER byte can hold anything. So this refuses a reordered word, a
// reversed bit order, a tail the implementation does not keep clean, and -- bit_cast handing back the object
// representation rather than the value -- a big-endian target, where bit n of a word lands at the far end of it.
// The endianness guard IS this probe, and there is no second one to fall out of step with it.
//
// NOT noexcept, and deliberately so: a bitset reading's set(pos) throws out_of_range for a position it does not
// have. The loop below never asks for one -- it skips i >= N, and the concept below settles the width before it
// gets here -- but that is reasoning a call graph cannot follow, and a throw out of a noexcept function is a
// terminate rather than a diagnostic. There is nothing to buy back either: every call to this is a constant
// evaluation, and a throw there is already a hard error.
template<class B, std::size_t N>
[[nodiscard]] constexpr auto bit_layout_holds()
        -> bool
{
        for (auto const byte : object_bytes(B())) {
                if (byte != std::byte{}) {
                        return false;
                }
        }
        for (auto const i : { 0UZ, 7UZ, bits_per_byte, bits_per_word, N - 1UZ }) {
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

// The second family: a field of bits whose layout is proved rather than believed. The size window bounds the
// object at its own positions plus at most one spare word, which catches a layout carrying a slot the five fixed
// positions never reach. Order is load-bearing throughout -- atomic constraints are checked left to right, so
// the cheap structural questions come before the probe, and bit_cast_is_constant comes before the probe that
// uses it.
template<class B, std::size_t N>
concept container_source =
        probeable_bits<B> and
        std::is_trivially_copyable_v<B> and
        // ITS width, not merely one that fits. Without this the size window below admits a neighbour -- a
        // std::bitset<9> is eight bytes, so it clears every bound a width of eight sets -- and eight of its nine
        // positions would convert while the ninth vanished. Nothing truncates is a property, so it is a constraint.
        // It also comes BEFORE the probe, which is what keeps the probe from asking a narrower source for a
        // position it does not have: std::bitset<8>::set(8) throws, and a throw is no constant expression, so that
        // would be a hard error where this is an unsatisfied concept.
        B().size() == N and
        sizeof(B) * bits_per_byte >= N and
        sizeof(B) * bits_per_byte <  N + bits_per_word and
        // With no byte to exchange there is nothing to prove, and asking anyway refuses the one width where the
        // question is empty: std::bitset<0> occupies a byte that represents no position, so a bit_cast of it reads
        // an uninitialised one and is no constant expression. The guard is byte_count and not N, because they are
        // zero together and byte_count is what the two functions below actually range over.
        (byte_count<N> == 0UZ or (bit_cast_is_constant<B> and bit_layout_holds<B, N>()))
;

template<class B, std::size_t N>
concept bit_castable = integer_source<B, N> or container_source<B, N>;

template<std::size_t N, class B>
        requires bit_castable<B, N>
[[nodiscard]] constexpr auto bit_bytes(B const& b) noexcept
        -> std::array<std::byte, byte_count<N>>
{
        auto bytes = std::array<std::byte, byte_count<N>>();
        if constexpr (byte_count<N> > 0UZ) {
                if constexpr (integer_source<B, N>) {
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(b >> (bits_per_byte * j)));
                        }
                } else {
                        auto const object = object_bytes(b);
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = object[j];
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
                auto value = B();
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        auto const byte = static_cast<B>(std::to_integer<unsigned char>(bytes[j]));
                        value = static_cast<B>(value | static_cast<B>(byte << (bits_per_byte * j)));
                }
                return value;
        } else {
                auto object = std::array<std::byte, sizeof(B)>();
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        object[j] = bytes[j];
                }
                return std::bit_cast<B>(object);
        }
}

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_BIT_CASTABLE_HPP
