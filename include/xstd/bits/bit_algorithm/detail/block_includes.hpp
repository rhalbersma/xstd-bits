//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_INCLUDES_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_INCLUDES_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd::bits::detail {

// std::ranges::includes on one block: every position of rhs is one of lhs, so rhs outside lhs is a zero Block.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto block_includes(Block lhs, Block rhs) noexcept
        -> bool
{
        return (rhs & static_cast<Block>(~lhs)) == static_cast<Block>(0);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_INCLUDES_HPP
