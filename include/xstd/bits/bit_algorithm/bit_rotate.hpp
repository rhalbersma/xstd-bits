//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // reverse_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, whole_sequence, writable_bools
#include <cassert>                                            // assert
#include <cstddef>                                            // size_t
#include <ranges>                                             // begin, borrowed_subrange_t, end, iterator_t, size, subrange

namespace xstd::ranges {

// std::ranges::rotate(r, middle), a block at a time: bit i takes bit (i + n) % size(), for n = middle - begin.
template<bits::detail::writable_bools R>
constexpr auto bit_rotate(R&& r, std::ranges::iterator_t<R> middle) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        using bits::detail::storage_access;
        auto const first = std::ranges::begin(r);
        auto const last  = std::ranges::end(r);
        assert(first <= middle and middle <= last);
        auto const n = static_cast<std::size_t>(middle - first);
        if constexpr (bits::detail::whole_sequence<R> and requires { storage_access::bits(r).rotate(n); }) {
                storage_access::bits(r).rotate(n);
        } else {
                // Each part reversed, then the whole: what stood in [middle, end) comes out in front, in its order.
                auto&& c     = storage_access::bits(r);
                auto const s = bits::detail::first_position(r);
                auto const z = std::ranges::size(r);
                bits::detail::reverse_bools(c, s, n);
                bits::detail::reverse_bools(c, s + n, z - n);
                bits::detail::reverse_bools(c, s, z);
        }
        return std::ranges::borrowed_subrange_t<R>(first + (last - middle), last);
}

// std::ranges::rotate(first, middle, last), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_rotate(I first, I middle, S last) noexcept
        -> std::ranges::subrange<I>
{
        auto w              = bits::detail::iterator_window(first, last);
        auto const [lo, hi] = xstd::ranges::bit_rotate(w, std::ranges::begin(w) + (middle - first));
        return {bits::detail::rebased(first, lo), bits::detail::rebased(first, hi)};
}

} // namespace xstd::ranges

namespace xstd {

// std::rotate(first, middle, last): where the element at first ends up.
template<bits::detail::bool_iterator I>
        requires bits::detail::writable_bools<bits::detail::iterator_window_t<I>>
constexpr auto bit_rotate(I first, I middle, I last) noexcept
        -> I
{
        return xstd::ranges::bit_rotate(first, middle, last).begin();
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_ROTATE_HPP
