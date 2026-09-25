//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CAST_HPP
#define XSTD_BITS_BIT_BIT_CAST_HPP

#include <xstd/bits/detail/bit_castable.hpp>       // bit_bytes, bit_castable, byte_count, bytes_bits, container_source
#include <xstd/bits/detail/ownership.hpp>          // owned_storage
#include <xstd/bits/from_bit_storage.hpp>          // from_bit_storage
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <bit>                                     // bit_cast
#include <cstddef>                                 // byte, size_t
#include <span>                                    // dynamic_extent
#include <tuple>                                   // tuple_size, tuple_size_v
#include <type_traits>                             // bool_constant, remove_const_t

// Between any two things that have bit storage of one width: the blocks are copied, whatever reading each presents.
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

// An array of words by value: its width is its length times the word's digits.
template<class T>
concept word_array =
        requires {
                typename std::tuple_size<T>::type;
                typename T::value_type;
        } and
        xstd::unsigned_integer<typename T::value_type>;

template<class T>
concept has_constant_size = requires { typename std::bool_constant<(T().size(), true)>; };

// The width a type has bit storage of, or dynamic_extent where it has none of a width fixed at compile time.
template<class T>
[[nodiscard]] consteval auto bit_width_of() noexcept
        -> std::size_t
{
        if constexpr (xstd::unsigned_integer<T>) {
                return static_cast<std::size_t>(xstd::numeric_limits<T>::digits);
        } else if constexpr (packed_owner<T>) {
                return std::remove_const_t<typename owned_storage<std::remove_const_t<T>>::bits_type>::extent;
        } else if constexpr (packed_view<T>) {
                return std::remove_const_t<typename T::adapted_type>::extent;
        } else if constexpr (word_array<T>) {
                return std::tuple_size_v<T> * static_cast<std::size_t>(xstd::numeric_limits<typename T::value_type>::digits);
        } else if constexpr (has_constant_size<T>) {
                if constexpr (container_source<T, T().size()>) {
                        return T().size();
                } else {
                        return std::dynamic_extent;
                }
        } else {
                return std::dynamic_extent;
        }
}

template<class T>
inline constexpr auto bit_width_v = bit_width_of<T>();

} // namespace xstd::bits::detail

namespace xstd {

// T has bit storage of a width fixed at compile time: one of ours, a word or an array of them, or a field it proves.
template<class T>
concept bit_castable =
        bits::detail::bit_width_v<T> != std::dynamic_extent and
        (bits::detail::packed<T> or bits::detail::bit_castable<T, bits::detail::bit_width_v<T>>);

// std::bit_cast for bit storage: the widths agree exactly, the blocks are copied, and a view is never written into.
template<bit_castable To, bit_castable From>
        requires (not bits::detail::packed_view<To>) and (bits::detail::bit_width_v<To> == bits::detail::bit_width_v<From>)
[[nodiscard]] constexpr auto bit_cast(From const& from) noexcept
        -> To
{
        constexpr auto N = bits::detail::bit_width_v<From>;
        using bytes_type = std::array<unsigned char, bits::detail::byte_count<N>>;
        using raw_bytes_type = std::array<std::byte, bits::detail::byte_count<N>>;
        if constexpr (N == 0UZ) {
                return To();
        } else {
                auto const bytes = [&] -> bytes_type {
                        if constexpr (bits::detail::packed<From>) {
                                return from.template to_bits<bytes_type>();
                        } else {
                                return std::bit_cast<bytes_type>(bits::detail::bit_bytes<N>(from));
                        }
                }();
                if constexpr (bits::detail::packed<To>) {
                        return To(xstd::from_bit_storage, bytes);
                } else {
                        return bits::detail::bytes_bits<To, N>(std::bit_cast<raw_bytes_type>(bytes));
                }
        }
}

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_CAST_HPP
