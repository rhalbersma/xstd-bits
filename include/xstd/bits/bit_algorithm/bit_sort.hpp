//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // count_true, first_position, writable_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <ranges>                                       // borrowed_iterator_t, end, size

namespace xstd {

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

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SORT_HPP
