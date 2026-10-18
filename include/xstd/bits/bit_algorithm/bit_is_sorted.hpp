//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/find.hpp>      // find_value
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <ranges>                                       // size

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

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_IS_SORTED_HPP
