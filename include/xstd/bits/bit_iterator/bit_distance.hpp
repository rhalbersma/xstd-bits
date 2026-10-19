//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP
#define XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bit_iterator
#include <xstd/bits/bit_iterator/detail/select.hpp>     // keys_between
#include <iterator>                                     // iter_difference_t, random_access_iterator, ranges::distance
#include <ranges>                                       // begin, common_range, end, iterator_t, range_difference_t, sized_range

namespace xstd::ranges {

// std::ranges::distance(first, last): an offset difference on a sequence, a popcount of the keys between on a set.
template<bits::detail::bit_iterator I>
[[nodiscard]] constexpr auto bit_distance(I first, I last) noexcept
        -> std::iter_difference_t<I>
{
        if constexpr (std::random_access_iterator<I>) {
                return std::ranges::distance(first, last);
        } else {
                return bits::detail::keys_between(first, last);
        }
}

// std::ranges::distance(r): the size where r knows it, and otherwise the distance between its ends.
template<std::ranges::common_range R>
        requires bits::detail::bit_iterator<std::ranges::iterator_t<R>>
[[nodiscard]] constexpr auto bit_distance(R&& r) noexcept
        -> std::ranges::range_difference_t<R>
{
        if constexpr (std::ranges::sized_range<R>) {
                return std::ranges::distance(r);
        } else {
                return xstd::ranges::bit_distance(std::ranges::begin(r), std::ranges::end(r));
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP
