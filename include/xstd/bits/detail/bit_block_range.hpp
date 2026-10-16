//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIT_BLOCK_RANGE_HPP
#define XSTD_BITS_DETAIL_BIT_BLOCK_RANGE_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <ranges>                                  // contiguous_range, range_reference_t, range_size_t, sized_range
#include <type_traits>                             // is_volatile_v, remove_reference_t

namespace xstd::bits::detail {

// One block, const or not but never volatile: block algorithms copy and merge the accesses a register keeps apart.
template<class Block>
concept bit_block = xstd::unsigned_integer<Block> and (not std::is_volatile_v<Block>);

// A sized contiguous range of such blocks that subscripts, asked of the reference, as range_value_t drops the cv.
template<class Blocks>
concept bit_block_range =
        std::ranges::sized_range<Blocks> and std::ranges::contiguous_range<Blocks> and
        bit_block<std::remove_reference_t<std::ranges::range_reference_t<Blocks>>> and
        requires (Blocks& blocks, std::ranges::range_size_t<Blocks> n) { blocks[n]; };

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BIT_BLOCK_RANGE_HPP
