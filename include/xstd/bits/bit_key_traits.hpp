//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_KEY_TRAITS_HPP
#define XSTD_BITS_BIT_KEY_TRAITS_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

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

} // namespace xstd

#endif // XSTD_BITS_BIT_KEY_TRAITS_HPP
