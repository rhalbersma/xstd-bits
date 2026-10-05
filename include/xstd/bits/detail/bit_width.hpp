//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_WIDTH_HPP
#define XSTD_BITS_DETAIL_BIT_WIDTH_HPP

#include <xstd/bits/bit_blocks.hpp>        // bit_blocks_extent_v, owned_bit_blocks
#include <xstd/bits/detail/bit_layout.hpp> // fixed_bit_blocks
#include <xstd/bits/detail/ownership.hpp>  // owned_storage
#include <bitset>                          // bitset
#include <cstddef>                         // size_t
#include <span>                            // dynamic_extent
#include <type_traits>                     // is_bounded_array_v, remove_const_t

// The width of bit storage a type has, fixed by its type: what xstd::bit_convert matches two fixed widths by.
namespace xstd::bits::detail {

// One of our owners, of any reading: its storage is named by the owner protocol.
template<class T>
concept packed_owner = requires { typename owned_storage<std::remove_const_t<T>>::bits_type; };

// One of our views over the whole width: a window starts inside a block, so its bits are not its storage's.
template<class T>
concept packed_view =
        (not packed_owner<T>) and
        requires { typename T::adapted_type; } and
        (not requires { requires T::is_windowed; });

template<class T>
concept packed = packed_owner<T> or packed_view<T>;

// A foreign type names its width only as std::bitset does, in its type; any other is read where a target supplies one.
template<class T>
inline constexpr auto foreign_bit_width = std::dynamic_extent;

template<std::size_t N>
inline constexpr auto foreign_bit_width<std::bitset<N>> = N;

// The width a type has bit storage of, or dynamic_extent where it has none of a width fixed at compile time.
template<class T>
[[nodiscard]] consteval auto bit_width_of() noexcept
        -> std::size_t
{
        if constexpr (packed_owner<T>) {
                return std::remove_const_t<typename owned_storage<std::remove_const_t<T>>::bits_type>::extent;
        } else if constexpr (packed_view<T>) {
                return std::remove_const_t<typename T::adapted_type>::extent;
        } else if constexpr (fixed_bit_blocks<T> and (std::is_bounded_array_v<T> or xstd::owned_bit_blocks<std::remove_const_t<T>>)) {
                // Held by value, so a span of a static extent stays out: it lends its width rather than having it.
                return xstd::bit_blocks_extent_v<T>;
        } else {
                return foreign_bit_width<std::remove_const_t<T>>;
        }
}

template<class T>
inline constexpr auto bit_width_v = bit_width_of<T>();

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_WIDTH_HPP
