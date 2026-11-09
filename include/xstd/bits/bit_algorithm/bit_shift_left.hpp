//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_LEFT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_LEFT_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // copy_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, whole_sequence, writable_bools
#include <cassert>                                            // assert
#include <cstddef>                                            // size_t
#include <iterator>                                           // iter_difference_t
#include <ranges>                                             // begin, borrowed_subrange_t, end, range_difference_t, subrange

namespace xstd::ranges {

// std::ranges::shift_left(r, n), a block at a time: bit i takes bit i + n, and the n past the new end read false.
template<bits::detail::writable_bools R>
constexpr auto bit_shift_left(R&& r, std::ranges::range_difference_t<R> n) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        using bits::detail::storage_access;
        assert(0 <= n);
        auto const first = std::ranges::begin(r);
        auto const last  = std::ranges::end(r);
        if (n == 0) {
                return {first, last};
        }
        // No effects at a shift past the end, as the standard's has none; the storage's own needs n < size().
        if (last - first <= n) {
                return {first, first};
        }
        auto const m = static_cast<std::size_t>(n);
        if constexpr (bits::detail::whole_sequence<R> and requires { storage_access::bits(r) >>= m; }) {
                storage_access::bits(r) >>= m;
        } else {
                // Lowest first, so each block is read before the one written below it can reach it.
                auto&& c     = storage_access::bits(r);
                auto const s = bits::detail::first_position(r);
                auto const z = static_cast<std::size_t>(last - first);
                bits::detail::copy_bools(c, s, c, s + m, z - m);
                c.set(s + z - m, m, false);
        }
        return {first, last - n};
}

// std::ranges::shift_left(first, last, n), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_shift_left(I first, S last, std::iter_difference_t<I> n) noexcept
        -> std::ranges::subrange<I>
{
        auto w              = bits::detail::iterator_window(first, last);
        auto const [lo, hi] = xstd::ranges::bit_shift_left(w, n);
        return {bits::detail::rebased(first, lo), bits::detail::rebased(first, hi)};
}

} // namespace xstd::ranges

namespace xstd {

// std::shift_left(first, last, n): the end of what moved.
template<bits::detail::bool_iterator I>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_shift_left(I first, I last, std::iter_difference_t<I> n) noexcept
        -> I
{
        return ranges::bit_shift_left(first, last, n).end();
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_LEFT_HPP
