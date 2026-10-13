//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_SELECT_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_SELECT_HPP

#include <xstd/bits/detail/intrin.hpp>             // countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cassert>                                 // assert
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// Select on one block: the position of its k-th set bit, from zero at the lowest, by clearing the k below it.
template<xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto block_select(Block block, std::size_t k) noexcept
        -> std::size_t
{
        assert(k < popcount(block));
        for ([[maybe_unused]] auto const i : std::views::iota(0UZ, k)) {
                block = static_cast<Block>(block & static_cast<Block>(block - Block{1}));
        }
        return countr_zero(block);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_BLOCK_SELECT_HPP
