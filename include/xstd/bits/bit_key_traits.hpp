//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_KEY_TRAITS_HPP
#define XSTD_BITS_BIT_KEY_TRAITS_HPP

#include <cstddef> // size_t

// How a set owner's key maps onto a position and back, as std::char_traits says what a character is.
namespace xstd {

// to_index preserves order, a < b exactly where to_index(a) < to_index(b), and from_index inverts it.
template<class Key>
struct bit_key_traits;

// The identity, with no size: a std::size_t key is its own position, in a universe left open.
template<>
struct bit_key_traits<std::size_t>
{
        [[nodiscard]] static constexpr auto to_index(std::size_t key) noexcept
                -> std::size_t
        {
                return key;
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> std::size_t
        {
                return index;
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_KEY_TRAITS_HPP
