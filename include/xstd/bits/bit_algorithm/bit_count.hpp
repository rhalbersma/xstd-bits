//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP

#include <xstd/bits/bit_algorithm/detail/count.hpp>           // count_true
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools
#include <iterator>                                           // iter_difference_t
#include <ranges>                                             // range_difference_t, size

namespace xstd::ranges {

// std::ranges::count(r, value), a block at a time: the ones popped, and the zeros as the size less the ones.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_count(R const& r, bool value) noexcept
        -> std::ranges::range_difference_t<R const>
{
        auto const ones = bits::detail::count_true(r);
        return static_cast<std::ranges::range_difference_t<R const>>(value ? ones : std::ranges::size(r) - ones);
}

// std::ranges::count(first, last, value), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
[[nodiscard]] constexpr auto bit_count(I first, S last, bool value) noexcept
        -> std::iter_difference_t<I>
{
        return static_cast<std::iter_difference_t<I>>(xstd::ranges::bit_count(bits::detail::iterator_window(first, last), value));
}

} // namespace xstd::ranges

namespace xstd {

// std::count(first, last, value).
template<bits::detail::bool_iterator I>
[[nodiscard]] constexpr auto bit_count(I first, I last, bool value) noexcept
        -> std::iter_difference_t<I>
{
        return xstd::ranges::bit_count(first, last, value);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP
