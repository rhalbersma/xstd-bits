//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_COUNT_HPP
#define XSTD_BITS_ALGORITHM_BIT_COUNT_HPP

#include <xstd/bits/detail/algorithm.hpp> // count_true, reads_bools
#include <ranges>                         // range_difference_t

namespace xstd {

// std::ranges::count(r, true), a block at a time.
template<bits::detail::reads_bools R>
[[nodiscard]] constexpr auto bit_count(R const& r) noexcept
        -> std::ranges::range_difference_t<R const>
{
        return static_cast<std::ranges::range_difference_t<R const>>(bits::detail::count_true(r));
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_COUNT_HPP
