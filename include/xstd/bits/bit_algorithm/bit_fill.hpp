//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // first_position, whole_sequence, writable_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <ranges>                                       // borrowed_iterator_t, end, size

namespace xstd {

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

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_FILL_HPP
