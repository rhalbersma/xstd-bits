//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // comparable_readings, reads_keys
#include <xstd/bits/bit_algorithm/detail/compare.hpp>   // three_way_bools, three_way_keys
#include <compare>                                      // strong_ordering

namespace xstd {

// std::lexicographical_compare_three_way over two of one reading's elements, a block at a time, as std::set orders.
template<class R1, class R2>
        requires bits::detail::comparable_readings<R1, R2>
[[nodiscard]] constexpr auto bit_lexicographical_compare_three_way(R1 const& r1, R2 const& r2) noexcept
        -> std::strong_ordering
{
        if constexpr (bits::detail::reads_keys<R1>) {
                return bits::detail::three_way_keys(r1, r2);
        } else {
                return bits::detail::three_way_bools(r1, r2);
        }
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_LEXICOGRAPHICAL_COMPARE_THREE_WAY_HPP
