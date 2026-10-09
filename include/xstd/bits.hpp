//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The umbrella over every container but the ext column's, which stays out so boost::container::small_vector is opt-in.

// Shared by both readings.
#include <xstd/bits/bit.hpp>         // IWYU pragma: export; bit_convert, bit_convertible, bit_convertible_to
#include <xstd/bits/bit_blocks.hpp>  // IWYU pragma: export; bit_align, bit_blocks, bit_blocks_extent_v, bit_fast, bit_least, bit_underlying, owned_bit_blocks, resizable_bit_blocks, underlying_block_t
#include <xstd/bits/bit_hasher.hpp>  // IWYU pragma: export; bit_hash_append, bit_hasher
#include <xstd/bits/from_blocks.hpp> // IWYU pragma: export; bit_constructible_from, from_blocks, from_blocks_t

// The sequence reading.
#include <xstd/bits/bit_array.hpp>          // IWYU pragma: export; bit_array
#include <xstd/bits/bit_bounded_vector.hpp> // IWYU pragma: export; bit_bounded_vector
#include <xstd/bits/bit_vector.hpp>         // IWYU pragma: export; bit_vector
#include <xstd/bits/bit_span.hpp>           // IWYU pragma: export; bit_span
#include <xstd/bits/bit_subspan.hpp>        // IWYU pragma: export; bit_subspan

// The set reading.
#include <xstd/bits/bit_index_mapping.hpp> // IWYU pragma: export; bit_index_mapping, bit_mask_mapping, sized_bit_index_mapping
#include <xstd/bits/bit_key_mapping.hpp>   // IWYU pragma: export; bit_find_mapping, bit_key_mapping, bit_range_mapping, enum_traits
#include <xstd/bits/bit_fixed_set.hpp>     // IWYU pragma: export; bit_fixed_set
#include <xstd/bits/bit_enum_set.hpp>      // IWYU pragma: export; bit_enum_set
#include <xstd/bits/bit_bounded_set.hpp>   // IWYU pragma: export; bit_bounded_set
#include <xstd/bits/bit_set.hpp>           // IWYU pragma: export; bit_set
#include <xstd/bits/bit_set_view.hpp>      // IWYU pragma: export; bit_set_view

// Flag types, a mask word spelled as the bitmask enumeration it replaces.
#include <xstd/bits/bit_flag_mapping.hpp> // IWYU pragma: export; bit_flag_mapping
#include <xstd/bits/bit_flag_set.hpp>     // IWYU pragma: export; bit_flag_set

#endif // XSTD_BITS_HPP
