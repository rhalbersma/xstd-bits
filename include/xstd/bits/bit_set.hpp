//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

// The set reading.
#include <xstd/bits/bit_set/bit_key_mapping.hpp> // IWYU pragma: export; bit_find_mapping, bit_key_mapping, bit_range_mapping, enum_traits
#include <xstd/bits/bit_set/bit_fixed_set.hpp>   // IWYU pragma: export; bit_fixed_set
#include <xstd/bits/bit_set/bit_enum_set.hpp>    // IWYU pragma: export; bit_enum_set
#include <xstd/bits/bit_set/bit_inplace_set.hpp> // IWYU pragma: export; bit_inplace_set
#include <xstd/bits/bit_set/bit_set.hpp>         // IWYU pragma: export; bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>    // IWYU pragma: export; bit_set_view

// Flag types, a mask block spelled as the bitmask enumeration it replaces.
#include <xstd/bits/bit_set/bit_flag_mapping.hpp> // IWYU pragma: export; bit_flag_mapping
#include <xstd/bits/bit_set/bit_flag_set.hpp>     // IWYU pragma: export; bit_flag_set

#endif // XSTD_BITS_BIT_SET_HPP
