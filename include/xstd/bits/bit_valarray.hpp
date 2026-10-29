//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_HPP
#define XSTD_BITS_BIT_VALARRAY_HPP

// [template.valarray], class template valarray, beside it the fixed and inplace columns
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // IWYU pragma: export; basic_bit_valarray, bit_valarray
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // IWYU pragma: export; basic_bit_fixed_valarray, bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_inplace_valarray.hpp> // IWYU pragma: export; basic_bit_inplace_valarray, bit_inplace_valarray

// [class.slice], class slice
#include <xstd/bits/bit_valarray/bit_slice.hpp> // IWYU pragma: export; bit_slice

// [template.slice.array], class template slice_array
#include <xstd/bits/bit_valarray/bit_slice_array.hpp> // IWYU pragma: export; bit_slice_array

// [class.gslice], class gslice
#include <xstd/bits/bit_valarray/bit_gslice.hpp> // IWYU pragma: export; bit_gslice

// [template.gslice.array], class template gslice_array
#include <xstd/bits/bit_valarray/bit_gslice_array.hpp> // IWYU pragma: export; bit_gslice_array

// [template.mask.array], class template mask_array
#include <xstd/bits/bit_valarray/bit_mask_array.hpp> // IWYU pragma: export; bit_mask_array

// [template.indirect.array], class template indirect_array
#include <xstd/bits/bit_valarray/bit_indirect_array.hpp> // IWYU pragma: export; bit_indirect_array

#endif // XSTD_BITS_BIT_VALARRAY_HPP
