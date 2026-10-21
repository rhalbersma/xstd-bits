//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP

#include <xstd/bits/bit_algorithm/detail/count.hpp> // count_true
#include <xstd/bits/detail/ownership.hpp>           // reads_bools
#include <ranges>                                   // range_difference_t, size

namespace xstd::ranges {

// std::ranges::count(r, value), a block at a time: the ones popped, and the zeros as the size less the ones.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_count(R const& r, bool value) noexcept
        -> std::ranges::range_difference_t<R const>
{
        auto const ones = bits::detail::count_true(r);
        return static_cast<std::ranges::range_difference_t<R const>>(value ? ones : std::ranges::size(r) - ones);
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_COUNT_HPP
