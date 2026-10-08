//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp>       // bit_block
#include <xstd/bits/bit_concepts/bit_block_range.hpp> // bit_block_range

// What every container and view here presents a packed interface over: bits in contiguous unsigned blocks.
namespace xstd {

// One block, or a range of them: what every container and view here holds its bits in.
template<class Bits>
concept bit_blocks = bit_block<Bits> or bit_block_range<Bits>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
