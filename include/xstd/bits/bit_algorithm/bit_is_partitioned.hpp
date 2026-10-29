//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_IS_PARTITIONED_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_IS_PARTITIONED_HPP

#include <xstd/bits/bit_algorithm/detail/find.hpp>     // find_value
#include <xstd/bits/bit_algorithm/detail/sequence.hpp> // first_position
#include <xstd/bits/detail/ownership.hpp>              // reads_bools, storage_access
#include <ranges>                                      // size

namespace xstd::ranges {

// std::ranges::is_partitioned(r, std::identity()), a block at a time: no true after the first false.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_is_partitioned(R const& r) noexcept
        -> bool
{
        auto const& c    = bits::detail::storage_access::bits(r);
        auto const first = bits::detail::first_position(r);
        auto const n     = std::ranges::size(r);
        auto const zero  = bits::detail::find_value(c, first, n, false);
        return bits::detail::find_value(c, first + zero, n - zero, true) == n - zero;
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_IS_PARTITIONED_HPP
