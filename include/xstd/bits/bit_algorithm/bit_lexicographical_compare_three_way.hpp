//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP

#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp>         // bool_sentinel_for, iterator_window
#include <xstd/bits/detail/bit_lexicographical_compare_three_way.hpp> // IWYU pragma: export; bit_lexicographical_compare_three_way
#include <xstd/bits/detail/iterator_concepts.hpp>                     // bool_iterator
#include <compare>                                                    // strong_ordering

namespace xstd::ranges {

// std::lexicographical_compare_three_way(first1, last1, first2, last2), over both pairs' positions as windows.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_iterator I2, bits::detail::bool_sentinel_for<I2> S2>
[[nodiscard]] constexpr auto bit_lexicographical_compare_three_way(I1 first1, S1 last1, I2 first2, S2 last2) noexcept
        -> std::strong_ordering
{
        return xstd::ranges::bit_lexicographical_compare_three_way(bits::detail::iterator_window(first1, last1), bits::detail::iterator_window(first2, last2));
}

} // namespace xstd::ranges

namespace xstd {

// std::lexicographical_compare_three_way(first1, last1, first2, last2), whose only form takes iterators.
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
[[nodiscard]] constexpr auto bit_lexicographical_compare_three_way(I1 first1, I1 last1, I2 first2, I2 last2) noexcept
        -> std::strong_ordering
{
        return xstd::ranges::bit_lexicographical_compare_three_way(first1, last1, first2, last2);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP
