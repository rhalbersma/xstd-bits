//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The umbrella over every container but the ext column's, which stays out so boost::container::small_vector is opt-in.

// Shared by both readings.
#include <xstd/bits/bit_concepts.hpp>    // IWYU pragma: export; bit_blocks, bit_constructible_from, bit_convertible_to, bit_index_mapping, bit_mask_mapping, sized_bit_index_mapping
#include <xstd/bits/bit_type_traits.hpp> // IWYU pragma: export; bit_align, bit_blocks_extent_v, bit_fast, bit_least, bit_rebind, bit_resize, bit_underlying, fast_block_t, least_block_t, underlying_block_t
#include <xstd/bits/from_blocks.hpp>     // IWYU pragma: export; from_blocks, from_blocks_t
#include <xstd/bits/bit.hpp>             // IWYU pragma: export; bit_convert
#include <xstd/bits/bit_functional.hpp>  // IWYU pragma: export; bit_equal_to, bit_hash_append, bit_hasher
#include <xstd/bits/bit_algorithm.hpp>   // IWYU pragma: export; bit_advance, bit_all_of, bit_any_of, bit_copy, bit_copy_backward, bit_count, bit_disjoint, bit_distance, bit_equal, bit_fill, bit_find, bit_find_last, bit_for_each, bit_includes, bit_intersects, bit_is_partitioned, bit_is_sorted, bit_lexicographical_compare_three_way, bit_mismatch, bit_move, bit_next, bit_none_of, bit_partition, bit_partition_point, bit_prev, bit_reverse, bit_reverse_copy, bit_rotate, bit_rotate_copy, bit_set_difference, bit_set_intersection, bit_set_symmetric_difference, bit_set_union, bit_shift_left, bit_shift_right, bit_sort, bit_swap_ranges, bit_transform

// The sequence reading.
#include <xstd/bits/bit_array.hpp>          // IWYU pragma: export; bit_array
#include <xstd/bits/bit_inplace_vector.hpp> // IWYU pragma: export; bit_inplace_vector
#include <xstd/bits/bit_vector.hpp>         // IWYU pragma: export; bit_vector
#include <xstd/bits/bit_span.hpp>           // IWYU pragma: export; bit_span
#include <xstd/bits/bit_subspan.hpp>        // IWYU pragma: export; bit_subspan

// The valarray reading.
#include <xstd/bits/bit_valarray.hpp> // IWYU pragma: export; bit_fixed_valarray, bit_gslice, bit_gslice_array, bit_indirect_array, bit_inplace_valarray, bit_mask_array, bit_slice, bit_slice_array, bit_valarray

// The set reading, flag types included.
#include <xstd/bits/bit_set.hpp> // IWYU pragma: export; bit_enum_set, bit_find_mapping, bit_fixed_set, bit_flag_mapping, bit_flag_set, bit_inplace_set, bit_key_mapping, bit_range_mapping, bit_set, bit_set_view, enum_traits

#endif // XSTD_BITS_HPP
