//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_PRED_HPP
#define XSTD_BITS_DETAIL_PRED_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::bits::detail {

// Both masks compare against a zero Block rather than convert to bool, which is the question each name asks.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto intersects(Block lhs, Block rhs) noexcept
        -> bool
{
        return (lhs & rhs) != static_cast<Block>(0);
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto is_subset_of(Block lhs, Block rhs) noexcept
        -> bool
{
        return (lhs & static_cast<Block>(~rhs)) == static_cast<Block>(0);
}

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto not_equal_to(Block lhs, Block rhs) noexcept
        -> bool
{
        return lhs != rhs;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_PRED_HPP
