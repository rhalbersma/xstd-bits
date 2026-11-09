//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP

#include <xstd/bits/bit_algorithm/detail/count.hpp>           // count_true
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, iterator_window_t, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, writable_bools
#include <ranges>                                             // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::sort(r), a block at a time: the ones counted, then the zeros filled in before them.
template<bits::detail::writable_bools R>
constexpr auto bit_sort(R&& r) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        auto const n     = std::ranges::size(r);
        auto const ones  = bits::detail::count_true(r);
        auto const first = bits::detail::first_position(r);
        bits::detail::storage_access::bits(r).set(first, n - ones, false);
        bits::detail::storage_access::bits(r).set(first + n - ones, ones, true);
        return std::ranges::borrowed_iterator_t<R>(std::ranges::end(r));
}

// std::ranges::sort(first, last), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_sort(I first, S last) noexcept
        -> I
{
        auto w = bits::detail::iterator_window(first, last);
        return bits::detail::rebased(first, xstd::ranges::bit_sort(w));
}

} // namespace xstd::ranges

namespace xstd {

// std::sort(first, last).
template<bits::detail::bool_iterator I>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_sort(I first, I last) noexcept
        -> void
{
        ranges::bit_sort(first, last);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP
