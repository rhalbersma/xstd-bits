//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BIT_CONVERT_HPP
#define XSTD_BITS_BIT_BIT_CONVERT_HPP

#include <xstd/bits/detail/bit_convertible.hpp> // adopt_blocks, adopts_from, bit_convertible, bit_target, convert_fixed, copy_blocks, fixed_width, foreign_convertible, narrow_blocks
#include <type_traits>                          // remove_cvref_t
#include <utility>                              // as_const, forward

// One conversion between everything that has bit storage, at any two widths: position i stays position i.
namespace xstd {

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

} // namespace xstd

#endif // XSTD_BITS_BIT_BIT_CONVERT_HPP
