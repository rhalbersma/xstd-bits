//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_KEY_TRAITS_HPP
#define XSTD_BITS_BIT_KEY_TRAITS_HPP

#include <xstd/bits/bit_enum_traits.hpp>           // bit_enum_traits, enum_traits
#include <xstd/ints/concepts/integer.hpp>          // integer
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/type_traits/make_unsigned.hpp> // make_unsigned_t
#include <cstddef>                                 // size_t
#include <type_traits>                             // is_enum_v

// How a set owner's key maps onto a position and back, as std::char_traits says what a character is.
namespace xstd {

// to_index preserves order, a < b exactly where to_index(a) < to_index(b), and from_index inverts it.
template<class Key>
struct bit_key_traits;

// The identity, with no size: an unsigned key is its own position, in a universe left open.
template<xstd::unsigned_integer Key>
struct bit_key_traits<Key>
{
        // A key wider than std::size_t must name a position: key <= std::numeric_limits<std::size_t>::max().
        [[nodiscard]] static constexpr auto to_index(Key key) noexcept
                -> std::size_t
        {
                return static_cast<std::size_t>(key);
        }

        // Only a position some key named comes back, so a narrower Key holds it.
        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                return static_cast<Key>(index);
        }
};

// An enumeration whose author listed its values: each enumerator's rank in that list, the universe closed at its size.
template<class Key>
        requires std::is_enum_v<Key> and requires { enum_traits<Key>::values; }
struct bit_key_traits<Key> : bit_enum_traits<Key>
{};

// The N keys from First at positions 0 through N - 1, for a signed key or a range not starting at 0.
template<xstd::integer Key, Key First, std::size_t N>
struct bit_offset_traits
{
        static constexpr auto size = N;

        // Modular, so the distance from the most negative First is exact rather than an overflow.
        [[nodiscard]] static constexpr auto to_index(Key key) noexcept
                -> std::size_t
        {
                return static_cast<std::size_t>(static_cast<unsigned_key>(static_cast<unsigned_key>(key) - static_cast<unsigned_key>(First)));
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                return static_cast<Key>(static_cast<unsigned_key>(static_cast<unsigned_key>(First) + static_cast<unsigned_key>(index)));
        }

private:
        // The outer cast undoes the promotion to int that a key narrower than int takes in the arithmetic.
        using unsigned_key = xstd::make_unsigned_t<Key>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_KEY_TRAITS_HPP
