//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CAST_HPP
#define XSTD_BITS_BIT_BIT_CAST_HPP

#include <xstd/bits/detail/bit_castable.hpp> // bit_bytes, bit_castable, byte_count, bytes_bits
#include <xstd/bits/detail/bit_width.hpp>    // bit_width_v, packed, packed_view
#include <xstd/bits/from_bit_storage.hpp>    // from_bit_storage
#include <array>                             // array
#include <bit>                               // bit_cast
#include <cstddef>                           // byte
#include <span>                              // dynamic_extent

// Between any two things that have bit storage of one width: the blocks are copied, whatever reading each presents.
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
