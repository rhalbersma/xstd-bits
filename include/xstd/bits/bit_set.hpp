//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_HPP

// [set], class template set
#include <xstd/bits/bit_set/bit_set.hpp> // IWYU pragma: export; basic_bit_set, bit_set

// beside it: the fixed-width and inplace-capacity sets, which <set> has no counterpart for
#include <xstd/bits/bit_set/bit_fixed_set.hpp>   // IWYU pragma: export; basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_inplace_set.hpp> // IWYU pragma: export; basic_bit_inplace_set, bit_inplace_set

// beside them: the tag that leaves a fixed set's blocks for a later assignment, as C++26's [[indeterminate]] does
#include <xstd/bits/bit_set/indeterminate.hpp> // IWYU pragma: export; indeterminate, indeterminate_array, indeterminate_t

// beside it: the set over borrowed blocks, which no standard header has
#include <xstd/bits/bit_set/bit_set_view.hpp> // IWYU pragma: export; bit_set_view

// beside it: the key mappings, the set's universe as key_compare is its order
#include <xstd/bits/bit_set/bit_key_mapping.hpp>  // IWYU pragma: export; bit_find_mapping, bit_key_mapping, bit_range_mapping, enum_traits
#include <xstd/bits/bit_set/bit_flag_mapping.hpp> // IWYU pragma: export; bit_flag_mapping

// beside it: the fixed sets an enumeration or a bitmask type names
#include <xstd/bits/bit_set/bit_enum_set.hpp> // IWYU pragma: export; bit_enum_set
#include <xstd/bits/bit_set/bit_flag_set.hpp> // IWYU pragma: export; bit_flag_set

// [set.erasure], erasure
#include <xstd/bits/bit_set/erase_if.hpp> // IWYU pragma: export; erase_if

// [unord.hash], beside it: std::hash, which <set> does not specialize for std::set
#include <xstd/bits/bit_set/hash.hpp> // IWYU pragma: export; hash

// Boost.ContainerHash's traits, https://www.boost.org/libs/container_hash
#include <xstd/bits/bit_set/container_hash.hpp> // IWYU pragma: export; is_range, is_tuple_like

#endif // XSTD_BITS_BIT_SET_HPP
