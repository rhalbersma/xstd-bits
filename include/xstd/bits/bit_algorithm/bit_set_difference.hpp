//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SET_DIFFERENCE_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SET_DIFFERENCE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // reads_keys
#include <concepts>                                     // same_as
#include <type_traits>                                  // type_identity_t
#include <utility>                                      // declval

namespace xstd::ranges {

// std::ranges::set_difference(s1, s2) as a set, which s1 - s2 spells; s2 converts to s1's type.
template<bits::detail::reads_keys S>
        requires requires (S const& s) { { s - s } -> std::same_as<S>; }
[[nodiscard]] constexpr auto bit_set_difference(S const& s1, std::type_identity_t<S> const& s2) noexcept(noexcept(std::declval<S const&>() - std::declval<S const&>()))
        -> S
{
        return s1 - s2;
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SET_DIFFERENCE_HPP
