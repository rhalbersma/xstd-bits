//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP

#include <xstd/bits/bit_algorithm/detail/count.hpp>           // count_true
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, iterator_window_t, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, writable_bools
#include <cstddef>                                            // ptrdiff_t
#include <ranges>                                             // begin, borrowed_subrange_t, end, size, subrange

namespace xstd::ranges {

// std::ranges::partition(r, std::identity()), a block at a time: the ones counted and filled in first, the zeros after.
template<bits::detail::writable_bools R>
constexpr auto bit_partition(R&& r) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        auto const n     = std::ranges::size(r);
        auto const ones  = bits::detail::count_true(r);
        auto const first = bits::detail::first_position(r);
        bits::detail::storage_access::bits(r).set(first, ones, true);
        bits::detail::storage_access::bits(r).set(first + ones, n - ones, false);
        return std::ranges::borrowed_subrange_t<R>(std::ranges::begin(r) + static_cast<std::ptrdiff_t>(ones), std::ranges::end(r));
}

// std::ranges::partition(first, last, std::identity()), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_partition(I first, S last) noexcept
        -> std::ranges::subrange<I>
{
        auto w              = bits::detail::iterator_window(first, last);
        auto const [lo, hi] = xstd::ranges::bit_partition(w);
        return {bits::detail::rebased(first, lo), bits::detail::rebased(first, hi)};
}

} // namespace xstd::ranges

namespace xstd {

// std::partition(first, last, std::identity()): the first false.
template<bits::detail::bool_iterator I>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_partition(I first, I last) noexcept
        -> I
{
        return ranges::bit_partition(first, last).begin();
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP
