//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP

#include <xstd/bits/bit_algorithm/detail/find.hpp>            // find_value
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position
#include <ranges>                                             // size

namespace xstd::ranges {

// std::ranges::is_sorted(r), a block at a time: no false after the first true.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_is_sorted(R const& r) noexcept
        -> bool
{
        auto const& c    = bits::detail::storage_access::bits(r);
        auto const first = bits::detail::first_position(r);
        auto const n     = std::ranges::size(r);
        auto const one   = bits::detail::find_value(c, first, n, true);
        return bits::detail::find_value(c, first + one, n - one, false) == n - one;
}

// std::ranges::is_sorted(first, last), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S>
[[nodiscard]] constexpr auto bit_is_sorted(I first, S last) noexcept
        -> bool
{
        return xstd::ranges::bit_is_sorted(bits::detail::iterator_window(first, last));
}

} // namespace xstd::ranges

namespace xstd {

// std::is_sorted(first, last).
template<bits::detail::bool_iterator I>
[[nodiscard]] constexpr auto bit_is_sorted(I first, I last) noexcept
        -> bool
{
        return xstd::ranges::bit_is_sorted(first, last);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP
