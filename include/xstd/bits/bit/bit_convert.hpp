//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CONVERT_HPP
#define XSTD_BITS_BIT_BIT_CONVERT_HPP

#include <xstd/bits/bit_storage.hpp>            // owned_bit_storage
#include <xstd/bits/detail/bit_convertible.hpp> // adopt_blocks, bit_convertible, bit_target, convert_fixed, copy_blocks, fixed_width, foreign_convertible, holds_whole_blocks, narrow_blocks
#include <xstd/bits/from_bit_storage.hpp>       // from_bit_storage_t
#include <concepts>                             // constructible_from, same_as
#include <type_traits>                          // is_const_v, is_rvalue_reference_v, remove_cvref_t, remove_reference_t
#include <utility>                              // as_const, declval, forward

// One conversion between everything that has bit storage, at any two widths: position i stays position i.
namespace xstd {

// Blocks that are bit storage for To as they are: the tag constructor takes them, and nothing is copied or shifted.
template<class To, class Blocks>
concept bit_constructible_from =
        xstd::owned_bit_storage<std::remove_cvref_t<Blocks>> and
        std::constructible_from<To, from_bit_storage_t, Blocks>;

namespace bits::detail {

// An rvalue whose blocks To takes as they are, all of them: they move rather than being copied.
template<class To, class From>
concept adopts_from =
        std::is_rvalue_reference_v<From&&> and
        (not std::is_const_v<std::remove_reference_t<From>>) and
        requires (From&& from) { std::forward<From>(from).extract(); } and
        bit_constructible_from<To, decltype(std::declval<From&&>().extract())> and
        holds_whole_blocks<To>;

} // namespace bits::detail

// The source's positions into To: equal fixed widths, a run-time width into a fixed one, anything into a run-time one.
template<class To, class From>
        requires bits::detail::bit_convertible<std::remove_cvref_t<From>, To>
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
