//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP

#include <xstd/bits/bit_type_traits/bit_rebind.hpp> // bit_rebind
#include <xstd/bits/detail/ordinal.hpp>             // ordinal_key, ordinal_t, unsigned_ordinal_t
#include <xstd/bits/detail/rebind.hpp>              // rebindable
#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <concepts>                                 // integral

namespace xstd {

// An enumeration's underlying type or an integer type made unsigned, the block a field or ABI of it already uses.
template<class Key>
        requires bits::detail::ordinal_key<Key> and std::integral<bits::detail::ordinal_t<Key>> and xstd::unsigned_integer<bits::detail::unsigned_ordinal_t<Key>>
using underlying_block_t = bits::detail::unsigned_ordinal_t<Key>;

// The same bit set in its key's underlying block, enumeration or integer, as an existing field or ABI stores it.
template<class Owner>
        requires bits::detail::rebindable<Owner> and requires { typename underlying_block_t<typename Owner::key_type>; }
using bit_underlying = bit_rebind<underlying_block_t<typename Owner::key_type>, Owner>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP
