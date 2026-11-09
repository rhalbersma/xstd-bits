//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_REVERSE_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_REVERSE_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // reverse_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, whole_sequence, writable_bools
#include <ranges>                                             // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::reverse(r), a block at a time: a whole sequence's storage end for end, a window's ends traded inward.
template<bits::detail::writable_bools R>
constexpr auto bit_reverse(R&& r) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        using bits::detail::storage_access;
        if constexpr (bits::detail::whole_sequence<R> and requires { storage_access::bits(r).reverse(); }) {
                storage_access::bits(r).reverse();
        } else {
                bits::detail::reverse_bools(storage_access::bits(r), bits::detail::first_position(r), std::ranges::size(r));
        }
        return std::ranges::borrowed_iterator_t<R>(std::ranges::end(r));
}

// std::ranges::reverse(first, last), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_reverse(I first, S last) noexcept
        -> I
{
        auto w = bits::detail::iterator_window(first, last);
        return bits::detail::rebased(first, xstd::ranges::bit_reverse(w));
}

} // namespace xstd::ranges

namespace xstd {

// std::reverse(first, last), beside xstd-ints' bit_reverse(x): both read the bits from the other end.
template<bits::detail::bool_iterator I>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_reverse(I first, I last) noexcept
        -> void
{
        xstd::ranges::bit_reverse(first, last);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_REVERSE_HPP
