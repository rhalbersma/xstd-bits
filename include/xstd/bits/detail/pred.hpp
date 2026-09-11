//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_PRED_HPP
#define XSTD_BITS_DETAIL_PRED_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::detail::bits {

// The cast is load-bearing for an integer-class Block. Returning lhs & rhs copy-initializes the bool, which
// takes an IMPLICIT conversion, and a 128-bit integer class offers only an explicit operator bool. The two
// predicates below never needed it: not and != both reach bool by a contextual conversion, which an explicit
// operator satisfies. [design.md#uint128-support]
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto intersects(Block lhs, Block rhs) noexcept
        -> bool
{
        return static_cast<bool>(lhs & rhs);
}   

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto is_subset_of(Block lhs, Block rhs) noexcept
        -> bool
{
        return not (lhs & static_cast<Block>(~rhs));
}  

template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto not_equal_to(Block lhs, Block rhs) noexcept
        -> bool
{
        return lhs != rhs;
}

}       // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_PRED_HPP
