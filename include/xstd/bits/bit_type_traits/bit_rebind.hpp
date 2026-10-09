//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_REBIND_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_REBIND_HPP

#include <xstd/bits/detail/rebind.hpp>             // rebind, rebindable
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer

namespace xstd {

// The same bit container over another block, allocator rebound too, as std::simd's rebind_t takes another element.
template<xstd::unsigned_integer Block, class Owner>
        requires bits::detail::rebindable<Owner>
using bit_rebind = bits::detail::rebind<Owner>::template with_block<Block>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_REBIND_HPP
