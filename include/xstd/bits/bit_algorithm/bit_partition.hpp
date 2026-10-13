//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // count_true, first_position, writable_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // begin, borrowed_subrange_t, end, size

namespace xstd {

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

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_HPP
