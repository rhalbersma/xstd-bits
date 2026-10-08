//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp> // bit_block
#include <xstd/bits/detail/rebind.hpp>          // rebind_t, rebind_width_v, rebindable
#include <xstd/ints/concepts/integer.hpp>       // integer
#include <concepts>                             // integral, same_as
#include <type_traits>                          // conditional_t, is_enum_v, make_unsigned_t, type_identity, underlying_type, underlying_type_t

namespace xstd {

// An enumeration's underlying type or an integer type made unsigned, the block a field or ABI of it already uses.
template<class Key>
        requires (std::is_enum_v<Key> and (not std::same_as<std::underlying_type_t<Key>, bool>) and bit_block<std::make_unsigned_t<std::underlying_type_t<Key>>>) or (std::integral<Key> and xstd::integer<Key>)
using underlying_block_t = std::make_unsigned_t<typename std::conditional_t<std::is_enum_v<Key>, std::underlying_type<Key>, std::type_identity<Key>>::type>;

// The same bit set in its key's underlying block, enumeration or integer, as an existing field or ABI stores it.
template<class Owner>
        requires bits::detail::rebindable<Owner> and requires { typename underlying_block_t<typename Owner::key_type>; }
using bit_underlying = bits::detail::rebind_t<Owner, underlying_block_t<typename Owner::key_type>, bits::detail::rebind_width_v<Owner>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_UNDERLYING_HPP
