//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ENUM_SET_HPP
#define XSTD_BITS_BIT_ENUM_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                // smallest_block_t
#include <xstd/bits/bit_enum_traits.hpp>           // bit_enum_traits
#include <xstd/bits/bit_fixed_set.hpp>             // basic_bit_fixed_set
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <type_traits>                             // is_enum_v

// The packed std::set<E> for an enumeration whose author listed its values, as Java's and Chromium's EnumSet are.
namespace xstd {

// One bit per listed value in the smallest block that holds them all; another block is the second argument.
template<class E, xstd::unsigned_integer Block = smallest_block_t<bit_enum_traits<E>::size>, class Traits = bit_enum_traits<E>>
        requires std::is_enum_v<E>
using bit_enum_set = basic_bit_fixed_set<E, Block, Traits::size, Traits>;

} // namespace xstd

#endif // XSTD_BITS_BIT_ENUM_SET_HPP
