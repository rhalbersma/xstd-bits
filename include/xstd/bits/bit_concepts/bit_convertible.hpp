//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_HPP

#include <xstd/bits/detail/bit_convertible.hpp> // bit_convert_source, fixed_target, fixed_width, foreign_convertible
#include <xstd/bits/detail/bit_width.hpp>       // bit_width_v
#include <xstd/bits/detail/ownership.hpp>       // owned_bits_t, owner
#include <concepts>                             // default_initializable
#include <type_traits>                          // is_const_v

namespace xstd {

// What bit_convert takes: equal fixed widths, run-time into fixed, anything into a run-time owner or foreign type.
template<class From, class To>
concept bit_convertible =
        (bits::detail::fixed_target<To> and bits::detail::fixed_width<From> and bits::detail::bit_width_v<To> == bits::detail::bit_width_v<From>) or
        (bits::detail::fixed_target<To> and bits::detail::bit_convert_source<From> and (not bits::detail::fixed_width<From>)) or
        (bits::detail::owner<To> and (not std::is_const_v<To>) and std::default_initializable<To> and bits::detail::owned_bits_t<To>::has_stored_size and bits::detail::bit_convert_source<From>) or
        bits::detail::foreign_convertible<To, From>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_CONVERTIBLE_HPP
