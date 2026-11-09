//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP

#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window
#include <xstd/bits/detail/bit_equal.hpp>                     // IWYU pragma: export; bit_equal
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator

namespace xstd::ranges {

// std::ranges::equal(first1, last1, first2, last2), over both pairs' positions as windows.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_iterator I2, bits::detail::bool_sentinel_for<I2> S2>
[[nodiscard]] constexpr auto bit_equal(I1 first1, S1 last1, I2 first2, S2 last2) noexcept
        -> bool
{
        return xstd::ranges::bit_equal(bits::detail::iterator_window(first1, last1), bits::detail::iterator_window(first2, last2));
}

} // namespace xstd::ranges

namespace xstd {

// std::equal(first1, last1, first2): the second range as long as the first.
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
[[nodiscard]] constexpr auto bit_equal(I1 first1, I1 last1, I2 first2) noexcept
        -> bool
{
        return xstd::ranges::bit_equal(first1, last1, first2, first2 + (last1 - first1));
}

// std::equal(first1, last1, first2, last2).
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
[[nodiscard]] constexpr auto bit_equal(I1 first1, I1 last1, I2 first2, I2 last2) noexcept
        -> bool
{
        return xstd::ranges::bit_equal(first1, last1, first2, last2);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP
