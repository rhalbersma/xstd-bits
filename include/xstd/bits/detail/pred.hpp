//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_PRED_HPP
#define XSTD_BITS_DETAIL_PRED_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::detail::bits {

// Both masks are compared against a zero Block rather than converted to a bool, which is the question each
// name asks: a conversion leaves the reader to translate "true" back into "shares a bit" or "has none outside".
// Zero is spelled as contiguous_bit_container spells it. The one spelling that does not work is returning the
// mask bare, which copy-initializes the bool and so takes an IMPLICIT conversion, where a 128-bit integer class
// offers only an explicit operator bool. [design.md#uint128-support]
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
