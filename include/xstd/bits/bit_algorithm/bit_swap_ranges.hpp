//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // swap_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, iterator_window_t, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, writable_bools
#include <algorithm>                                          // min, swap_ranges_result
#include <cstddef>                                            // ptrdiff_t
#include <ranges>                                             // begin, borrowed_iterator_t, size

namespace xstd::ranges {

// std::ranges::swap_ranges(r1, r2), a block at a time, as far as the shorter goes.
template<bits::detail::writable_bools R1, bits::detail::writable_bools R2>
constexpr auto bit_swap_ranges(R1&& r1, R2&& r2) noexcept
        -> std::ranges::swap_ranges_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
{
        using bits::detail::storage_access;
        auto const n = std::ranges::min(std::ranges::size(r1), std::ranges::size(r2));
        bits::detail::swap_bools(storage_access::bits(r1), bits::detail::first_position(r1), storage_access::bits(r2), bits::detail::first_position(r2), n);
        auto const d = static_cast<std::ptrdiff_t>(n);
        return {std::ranges::borrowed_iterator_t<R1>(std::ranges::begin(r1) + d), std::ranges::borrowed_iterator_t<R2>(std::ranges::begin(r2) + d)};
}

// std::ranges::swap_ranges(first1, last1, first2, last2), over both pairs' positions as windows.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_iterator I2, bits::detail::bool_sentinel_for<I2> S2>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I1>> and bits::detail::writable_bools<bits::detail::iterator_window_t<I2>>
constexpr auto bit_swap_ranges(I1 first1, S1 last1, I2 first2, S2 last2) noexcept
        -> std::ranges::swap_ranges_result<I1, I2>
{
        auto w1               = bits::detail::iterator_window(first1, last1);
        auto w2               = bits::detail::iterator_window(first2, last2);
        auto const [in1, in2] = xstd::ranges::bit_swap_ranges(w1, w2);
        return {bits::detail::rebased(first1, in1), bits::detail::rebased(first2, in2)};
}

} // namespace xstd::ranges

namespace xstd {

// std::swap_ranges(first1, last1, first2): the end of what was swapped in the second range.
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I1>> and bits::detail::writable_bools<bits::detail::iterator_window_t<I2>>
constexpr auto bit_swap_ranges(I1 first1, I1 last1, I2 first2) noexcept
        -> I2
{
        return xstd::ranges::bit_swap_ranges(first1, last1, first2, first2 + (last1 - first1)).in2;
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP
