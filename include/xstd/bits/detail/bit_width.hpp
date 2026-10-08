//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_WIDTH_HPP
#define XSTD_BITS_DETAIL_BIT_WIDTH_HPP

#include <xstd/bits/bit_concepts/owned_bit_blocks.hpp>     // owned_bit_blocks
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_layout.hpp>                 // fixed_bit_blocks
#include <xstd/bits/detail/ownership.hpp>                  // owned_storage, owner, view
#include <bitset>                                          // bitset
#include <cstddef>                                         // size_t
#include <span>                                            // dynamic_extent
#include <type_traits>                                     // is_bounded_array_v, remove_const_t

// The width of bit storage a type has, fixed by its type: what xstd::bit_convert matches two fixed widths by.
namespace xstd::bits::detail {

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
        if constexpr (owner<T>) {
                return std::remove_const_t<typename owned_storage<std::remove_const_t<T>>::bits_type>::extent;
        } else if constexpr (view<T> and (not requires { requires T::is_windowed; })) {
                // A window starts inside a block, so its bits are not its storage's, where a whole view's are.
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
