//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_RANGE_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_RANGE_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp> // bit_block
#include <ranges>                               // contiguous_range, range_size_t, range_value_t, sized_range

namespace xstd {

// A sized contiguous range of blocks that subscripts.
template<class Blocks>
concept bit_block_range =
        std::ranges::sized_range<Blocks> and std::ranges::contiguous_range<Blocks> and
        bit_block<std::ranges::range_value_t<Blocks>> and
        requires (Blocks& blocks, std::ranges::range_size_t<Blocks> n) { blocks[n]; };

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_RANGE_HPP
