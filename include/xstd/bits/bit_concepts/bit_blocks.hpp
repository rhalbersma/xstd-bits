//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP

#include <xstd/bits/detail/bit_block_range.hpp>    // bit_block_range
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <type_traits>                             // is_volatile_v

// What every container and view here presents a packed interface over: bits in contiguous unsigned blocks.
namespace xstd {

// One block, or a range of them, never volatile: block algorithms merge the accesses a register keeps apart.
template<class Bits>
concept bit_blocks = (xstd::unsigned_integer<Bits> and not std::is_volatile_v<Bits>) or bits::detail::bit_block_range<Bits>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_BLOCKS_HPP
