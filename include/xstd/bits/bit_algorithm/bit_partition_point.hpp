//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_POINT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_POINT_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/find.hpp>      // find_value
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // begin, borrowed_iterator_t, size

namespace xstd::ranges {

// std::ranges::partition_point(r, std::identity()) of a partitioned r, a block at a time: its first false.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_partition_point(R&& r) noexcept
        -> std::ranges::borrowed_iterator_t<R>
{
        auto const i = bits::detail::find_value(bits::detail::storage_access::bits(r), bits::detail::first_position(r), std::ranges::size(r), false);
        return std::ranges::borrowed_iterator_t<R>(std::ranges::begin(r) + static_cast<std::ptrdiff_t>(i));
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_PARTITION_POINT_HPP
