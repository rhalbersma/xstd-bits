//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_INTERSECTS_HPP
#define XSTD_BITS_DETAIL_BLOCK_INTERSECTS_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::bits::detail {

// Compared against a zero Block rather than converted to bool, which is the question the name asks.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto block_intersects(Block lhs, Block rhs) noexcept
        -> bool
{
        return (lhs & rhs) != static_cast<Block>(0);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCK_INTERSECTS_HPP
