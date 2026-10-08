//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd {

// One unsigned block, const where a view only reads.
template<class Block>
concept bit_block = xstd::unsigned_integer<Block>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_BLOCK_HPP
