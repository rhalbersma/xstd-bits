//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ENUM_SET_HPP
#define XSTD_BITS_BIT_ENUM_SET_HPP

#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_fixed_set.hpp>                        // basic_bit_fixed_set
#include <xstd/bits/bit_key_mapping.hpp>                      // bit_key_mapping
#include <xstd/bits/bit_type_traits/bit_least.hpp>            // bit_least
#include <cstddef>                                            // size_t
#include <type_traits>                                        // is_enum_v

// The packed std::set<Enum> for an enumeration with a sized default mapping, as Java's and Chromium's EnumSet are.
namespace xstd {

// The fixed set of an enumeration's keys, one bit each in the smallest block that holds them.
template<class Enum>
        requires std::is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
using bit_enum_set = bit_least<basic_bit_fixed_set<Enum, std::size_t, bit_key_mapping<Enum>::size, bit_key_mapping<Enum>>>;

} // namespace xstd

#endif // XSTD_BITS_BIT_ENUM_SET_HPP
