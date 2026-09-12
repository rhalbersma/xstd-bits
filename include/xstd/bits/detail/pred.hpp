//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_PRED_HPP
#define XSTD_BITS_DETAIL_PRED_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::detail::bits {

// Both masks are compared against a zero Block rather than converted to a bool, which is the question each name
// asks: a conversion leaves the reader to translate "true" back into "shares a bit" or "has none outside".
// std::bitset answers the same question the same way -- libstdc++'s _M_is_any and _Unchecked_test both read
// != static_cast<_WordT>(0) -- and so does contiguous_bit_container, at twenty-three sites and no other
// spelling, which is where the zero here is spelled from.
//
// The spelling that does not work is returning the mask bare: that copy-initializes the bool and so takes an
// IMPLICIT conversion, where unsigned_integer promises only a CONTEXTUAL one -- [iterator.concept.winc]/8,
// spelled constructible_from<bool, T>. absl::uint128 provides exactly that and no more, its operator bool
// being explicit, so it is CONFORMING and the bare return was asking for more than the concept guarantees.
// The builtins grant the stronger conversion, and boost::int128::uint128 grants it too by a different route
// -- a non-explicit operator UnsignedInteger whose bool case returns low || high -- which is why absl alone
// surfaced the assumption. [design.md#uint128-support]
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

}       // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_PRED_HPP
