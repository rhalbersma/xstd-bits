//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP

#include <xstd/bits/detail/bit_block_range.hpp> // bit_block, bit_block_range

// What every container and view here presents a packed interface over: bits in contiguous unsigned blocks.
namespace xstd {

// One block, or a range of them, const as the bare type is and never volatile, which a register's accesses must be.
template<class Bits>
concept bit_blocks = bits::detail::bit_block<Bits> or bits::detail::bit_block_range<Bits>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
