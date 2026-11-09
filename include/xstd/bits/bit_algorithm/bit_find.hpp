//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FIND_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FIND_HPP

#include <xstd/bits/bit_algorithm/detail/find.hpp>            // find_value
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position
#include <cstddef>                                            // ptrdiff_t
#include <ranges>                                             // begin, borrowed_iterator_t, size

namespace xstd::ranges {

// std::ranges::find(r, value), a block at a time: the first position holding value, or the end.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_find(R&& r, bool value) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        auto const i = bits::detail::find_value(bits::detail::storage_access::bits(r), bits::detail::first_position(r), std::ranges::size(r), value);
        return std::ranges::borrowed_iterator_t<R>(std::ranges::begin(r) + static_cast<std::ptrdiff_t>(i));
}

// std::ranges::find(first, last, value), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
[[nodiscard]] constexpr auto bit_find(I first, S last, bool value) noexcept
        -> I
{
        auto w = bits::detail::iterator_window(first, last);
        return bits::detail::rebased(first, xstd::ranges::bit_find(w, value));
}

} // namespace xstd::ranges

namespace xstd {

// std::find(first, last, value).
template<bits::detail::bool_iterator I>
[[nodiscard]] constexpr auto bit_find(I first, I last, bool value) noexcept
        -> I
{
        return ranges::bit_find(first, last, value);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FIND_HPP
