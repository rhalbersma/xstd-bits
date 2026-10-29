//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_HPP
#define XSTD_BITS_BIT_ALGORITHM_HPP

// [alg.nonmodifying], non-modifying sequence operations
#include <xstd/bits/bit_algorithm/bit_all_of.hpp>    // IWYU pragma: export; bit_all_of
#include <xstd/bits/bit_algorithm/bit_any_of.hpp>    // IWYU pragma: export; bit_any_of
#include <xstd/bits/bit_algorithm/bit_none_of.hpp>   // IWYU pragma: export; bit_none_of
#include <xstd/bits/bit_algorithm/bit_for_each.hpp>  // IWYU pragma: export; bit_for_each
#include <xstd/bits/bit_algorithm/bit_find.hpp>      // IWYU pragma: export; bit_find
#include <xstd/bits/bit_algorithm/bit_find_last.hpp> // IWYU pragma: export; bit_find_last
#include <xstd/bits/bit_algorithm/bit_count.hpp>     // IWYU pragma: export; bit_count
#include <xstd/bits/bit_algorithm/bit_mismatch.hpp>  // IWYU pragma: export; bit_mismatch
#include <xstd/bits/bit_algorithm/bit_equal.hpp>     // IWYU pragma: export; bit_equal

// [alg.modifying.operations], mutating sequence operations
#include <xstd/bits/bit_algorithm/bit_copy.hpp>          // IWYU pragma: export; bit_copy
#include <xstd/bits/bit_algorithm/bit_copy_backward.hpp> // IWYU pragma: export; bit_copy_backward
#include <xstd/bits/bit_algorithm/bit_move.hpp>          // IWYU pragma: export; bit_move
#include <xstd/bits/bit_algorithm/bit_swap_ranges.hpp>   // IWYU pragma: export; bit_swap_ranges
#include <xstd/bits/bit_algorithm/bit_transform.hpp>     // IWYU pragma: export; bit_transform
#include <xstd/bits/bit_algorithm/bit_fill.hpp>          // IWYU pragma: export; bit_fill
#include <xstd/bits/bit_algorithm/bit_reverse.hpp>       // IWYU pragma: export; bit_reverse
#include <xstd/bits/bit_algorithm/bit_reverse_copy.hpp>  // IWYU pragma: export; bit_reverse_copy
#include <xstd/bits/bit_algorithm/bit_rotate.hpp>        // IWYU pragma: export; bit_rotate
#include <xstd/bits/bit_algorithm/bit_rotate_copy.hpp>   // IWYU pragma: export; bit_rotate_copy
#include <xstd/bits/bit_algorithm/bit_shift_left.hpp>    // IWYU pragma: export; bit_shift_left
#include <xstd/bits/bit_algorithm/bit_shift_right.hpp>   // IWYU pragma: export; bit_shift_right

// [alg.sorting], sorting and related operations, with intersects and disjoint beside includes
#include <xstd/bits/bit_algorithm/bit_sort.hpp>                              // IWYU pragma: export; bit_sort
#include <xstd/bits/bit_algorithm/bit_is_sorted.hpp>                         // IWYU pragma: export; bit_is_sorted
#include <xstd/bits/bit_algorithm/bit_is_partitioned.hpp>                    // IWYU pragma: export; bit_is_partitioned
#include <xstd/bits/bit_algorithm/bit_partition.hpp>                         // IWYU pragma: export; bit_partition
#include <xstd/bits/bit_algorithm/bit_partition_point.hpp>                   // IWYU pragma: export; bit_partition_point
#include <xstd/bits/bit_algorithm/bit_includes.hpp>                          // IWYU pragma: export; bit_includes
#include <xstd/bits/bit_algorithm/bit_intersects.hpp>                        // IWYU pragma: export; bit_intersects
#include <xstd/bits/bit_algorithm/bit_disjoint.hpp>                          // IWYU pragma: export; bit_disjoint
#include <xstd/bits/bit_algorithm/bit_set_union.hpp>                         // IWYU pragma: export; bit_set_union
#include <xstd/bits/bit_algorithm/bit_set_intersection.hpp>                  // IWYU pragma: export; bit_set_intersection
#include <xstd/bits/bit_algorithm/bit_set_difference.hpp>                    // IWYU pragma: export; bit_set_difference
#include <xstd/bits/bit_algorithm/bit_set_symmetric_difference.hpp>          // IWYU pragma: export; bit_set_symmetric_difference
#include <xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp> // IWYU pragma: export; bit_lexicographical_compare_three_way

#endif // XSTD_BITS_BIT_ALGORITHM_HPP
