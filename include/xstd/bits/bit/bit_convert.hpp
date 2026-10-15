//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CONVERT_HPP
#define XSTD_BITS_BIT_BIT_CONVERT_HPP

#include <xstd/bits/bit/detail/bit_convertible.hpp> // adopt_blocks, adopts_from, bit_convertible, bit_target, convert_fixed, copy_blocks, fixed_width, foreign_convertible, narrow_blocks, read_once
#include <type_traits>                              // is_volatile_v, remove_cv_t, remove_cvref_t, remove_reference_t
#include <utility>                                  // as_const, forward

// One conversion between everything that has bit storage, at any two widths: position i stays position i.
namespace xstd {

// The source's positions into To: fixed into at least as wide, run-time into fixed, anything into a run-time width.
template<class To, class From>
        requires bits::detail::bit_convertible<std::remove_cvref_t<From>, std::remove_cv_t<To>> and bits::detail::read_once<From>
[[nodiscard]] constexpr auto bit_convert(From&& from) noexcept(bits::detail::fixed_width<std::remove_cv_t<To>> and bits::detail::fixed_width<std::remove_cvref_t<From>>)
        -> std::remove_cv_t<To> // cv-transparent: To's cv is dropped, as a cast's scalar prvalue drops it
{
        using target_type = std::remove_cv_t<To>;
        using source_type = std::remove_cvref_t<From>;
        if constexpr (std::is_volatile_v<std::remove_reference_t<From>>) {
                // A volatile scalar, a status register say, is read once and converted as the value it held.
                return xstd::bit_convert<target_type>(static_cast<source_type>(from));
        } else if constexpr (bits::detail::foreign_convertible<target_type, source_type>) {
                return bits::detail::bit_target<target_type>::convert(std::as_const(from));
        } else if constexpr (bits::detail::fixed_width<target_type> and bits::detail::fixed_width<source_type>) {
                return bits::detail::convert_fixed<target_type>(std::as_const(from));
        } else if constexpr (bits::detail::fixed_width<target_type>) {
                return bits::detail::narrow_blocks<target_type>(std::as_const(from));
        } else if constexpr (bits::detail::adopts_from<target_type, From>) {
                return bits::detail::adopt_blocks<target_type>(std::forward<From>(from));
        } else {
                return bits::detail::copy_blocks<target_type>(std::as_const(from));
        }
}

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_CONVERT_HPP
