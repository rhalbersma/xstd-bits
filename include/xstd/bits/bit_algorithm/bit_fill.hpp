//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP

#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, iterator_window_t, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, whole_sequence, writable_bools
#include <ranges>                                             // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::fill(r, value), a block at a time: a whole sequence's every block, a window's masked at its ends.
template<bits::detail::writable_bools R>
constexpr auto bit_fill(R&& r, bool value) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        if constexpr (bits::detail::whole_sequence<R>) {
                bits::detail::storage_access::bits(r).fill(value);
        } else {
                bits::detail::storage_access::bits(r).set(bits::detail::first_position(r), std::ranges::size(r), value);
        }
        return std::ranges::borrowed_iterator_t<R>(std::ranges::end(r));
}

// std::ranges::fill(first, last, value), over the positions [first, last) as a window.
template<bits::detail::bool_iterator O, bits::detail::bool_sentinel_for<O> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<O>>
constexpr auto bit_fill(O first, S last, bool value) noexcept
        -> O
{
        auto w = bits::detail::iterator_window(first, last);
        return bits::detail::rebased(first, xstd::ranges::bit_fill(w, value));
}

} // namespace xstd::ranges

namespace xstd {

// std::fill(first, last, value).
template<bits::detail::bool_iterator O>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<O>>
constexpr auto bit_fill(O first, O last, bool value) noexcept
        -> void
{
        xstd::ranges::bit_fill(first, last, value);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP
