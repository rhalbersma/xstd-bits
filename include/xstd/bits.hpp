//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The umbrella over every container the standard library can store; the ext column stays out, so Boost is opt-in.

// Shared by all three readings.
#include <xstd/bits/bit.hpp>              // IWYU pragma: export; bit_cast, bit_castable
#include <xstd/bits/bit_storage.hpp>      // IWYU pragma: export; bit_storage, bit_storage_extent_v, owned_bit_storage, resizable_bit_storage
#include <xstd/bits/from_bit_storage.hpp> // IWYU pragma: export; from_bit_storage, from_bit_storage_t

// The bitset reading.
#include <xstd/bits/bitset.hpp>         // IWYU pragma: export; bitset
#include <xstd/bits/bounded_bitset.hpp> // IWYU pragma: export; bounded_bitset
#include <xstd/bits/dynamic_bitset.hpp> // IWYU pragma: export; dynamic_bitset

// The sequence reading.
#include <xstd/bits/bit_array.hpp>          // IWYU pragma: export; bit_array
#include <xstd/bits/bit_bounded_vector.hpp> // IWYU pragma: export; bit_bounded_vector
#include <xstd/bits/bit_vector.hpp>         // IWYU pragma: export; bit_vector
#include <xstd/bits/bit_span.hpp>           // IWYU pragma: export; bit_span
#include <xstd/bits/bit_subspan.hpp>        // IWYU pragma: export; bit_subspan

// The set reading.
#include <xstd/bits/bit_fixed_set.hpp>   // IWYU pragma: export; bit_fixed_set
#include <xstd/bits/bit_bounded_set.hpp> // IWYU pragma: export; bit_bounded_set
#include <xstd/bits/bit_set.hpp>         // IWYU pragma: export; bit_set
#include <xstd/bits/bit_set_view.hpp>    // IWYU pragma: export; bit_set_view

#endif // XSTD_BITS_HPP
