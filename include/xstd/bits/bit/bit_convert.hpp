//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CONVERT_HPP
#define XSTD_BITS_BIT_BIT_CONVERT_HPP

#include <xstd/bits/detail/bit_convertible.hpp> // adopt_blocks, adopts_from, bit_convert_source, bit_target, convert_fixed, copy_blocks, fixed_target, fixed_width, foreign_convertible, narrow_blocks
#include <xstd/bits/detail/bit_width.hpp>       // bit_width_v
#include <xstd/bits/detail/ownership.hpp>       // owned_bits_t, owner
#include <xstd/bits/from_blocks.hpp>            // IWYU pragma: export; bit_constructible_from
#include <concepts>                             // default_initializable, same_as
#include <type_traits>                          // is_const_v, remove_cvref_t
#include <utility>                              // as_const, forward

// One conversion between everything that has bit storage, at any two widths: position i stays position i.
namespace xstd {

// What bit_convert takes: equal fixed widths, run-time into fixed, anything into a run-time owner or foreign type.
template<class From, class To>
concept bit_convertible =
        (bits::detail::fixed_target<To> and bits::detail::fixed_width<From> and bits::detail::bit_width_v<To> == bits::detail::bit_width_v<From>) or
        (bits::detail::fixed_target<To> and bits::detail::bit_convert_source<From> and (not bits::detail::fixed_width<From>)) or
        (bits::detail::owner<To> and (not std::is_const_v<To>) and std::default_initializable<To> and bits::detail::owned_bits_t<To>::has_stored_size and bits::detail::bit_convert_source<From>) or
        bits::detail::foreign_convertible<To, From>;

// The source's positions into To: equal fixed widths, a run-time width into a fixed one, anything into a run-time one.
template<class To, class From>
        requires bit_convertible<std::remove_cvref_t<From>, To>
[[nodiscard]] constexpr auto bit_convert(From&& from) noexcept(bits::detail::fixed_width<To> and bits::detail::fixed_width<std::remove_cvref_t<From>>)
        -> To
{
        using source_type = std::remove_cvref_t<From>;
        if constexpr (bits::detail::foreign_convertible<To, source_type>) {
                return bits::detail::bit_target<To>::convert(std::as_const(from));
        } else if constexpr (bits::detail::fixed_width<To> and bits::detail::fixed_width<source_type>) {
                return bits::detail::convert_fixed<To>(std::as_const(from));
        } else if constexpr (bits::detail::fixed_width<To>) {
                return bits::detail::narrow_blocks<To>(std::as_const(from));
        } else if constexpr (bits::detail::adopts_from<To, From>) {
                return bits::detail::adopt_blocks<To>(std::forward<From>(from));
        } else {
                return bits::detail::copy_blocks<To>(std::as_const(from));
        }
}

// Whether bit_convert<To>(from) is valid: the positions map, though a run-time source may still throw.
template<class From, class To>
concept bit_convertible_to = requires (From&& from) {
        { xstd::bit_convert<To>(std::forward<From>(from)) } -> std::same_as<To>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_CONVERT_HPP
