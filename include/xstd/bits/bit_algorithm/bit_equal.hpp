//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP

#include <xstd/bits/bit_algorithm/detail/comparable.hpp> // comparable_readings
#include <xstd/bits/bit_algorithm/detail/compare.hpp>    // equal_bools, equal_keys
#include <xstd/bits/detail/ownership.hpp>                // reads_keys

namespace xstd::ranges {

// std::ranges::equal(r1, r2) over two of one reading, a block at a time, whatever blocks and storage hold each.
template<class R1, class R2>
        requires bits::detail::comparable_readings<R1, R2>
[[nodiscard]] constexpr auto bit_equal(R1 const& r1, R2 const& r2) noexcept
        -> bool
{
        if constexpr (bits::detail::reads_keys<R1>) {
                return bits::detail::equal_keys(r1, r2);
        } else {
                return bits::detail::equal_bools(r1, r2);
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_EQUAL_HPP
