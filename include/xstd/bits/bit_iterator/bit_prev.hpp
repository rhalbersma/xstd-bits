//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_BIT_PREV_HPP
#define XSTD_BITS_BIT_ITERATOR_BIT_PREV_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bit_iterator
#include <xstd/bits/bit_iterator/bit_advance.hpp>       // bit_advance
#include <iterator>                                     // bidirectional_iterator, iter_difference_t

namespace xstd::ranges {

// std::ranges::prev(it, n), through bit_advance by -n.
template<bits::detail::bit_iterator I>
        requires std::bidirectional_iterator<I>
[[nodiscard]] constexpr auto bit_prev(I it, std::iter_difference_t<I> n) noexcept
        -> I
{
        xstd::ranges::bit_advance(it, -n);
        return it;
}

// std::ranges::prev(it, n, bound), stopping at bound.
template<bits::detail::bit_iterator I>
        requires std::bidirectional_iterator<I>
[[nodiscard]] constexpr auto bit_prev(I it, std::iter_difference_t<I> n, I bound) noexcept
        -> I
{
        static_cast<void>(xstd::ranges::bit_advance(it, -n, bound));
        return it;
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ITERATOR_BIT_PREV_HPP
