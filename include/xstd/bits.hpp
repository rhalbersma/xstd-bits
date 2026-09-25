//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The umbrella over every container the standard library can store; the ext column stays out, so Boost is opt-in.

// Shared by all three readings.
#include <xstd/bits/bit_storage.hpp>             // IWYU pragma: export; bit_storage
#include <xstd/bits/bit.hpp>                     // IWYU pragma: export; bit_cast, bit_castable
#include <xstd/bits/from_bit_storage.hpp>        // IWYU pragma: export; from_bit_storage, from_bit_storage_t
#include <xstd/bits/contiguous_bit_sequence.hpp> // IWYU pragma: export; contiguous_bit_sequence

// The bitset reading.
#include <xstd/bits/bitset_adaptor.hpp> // IWYU pragma: export; bitset_adaptor
#include <xstd/bits/bitset.hpp>         // IWYU pragma: export; bitset
#include <xstd/bits/inplace_bitset.hpp> // IWYU pragma: export; inplace_bitset
#include <xstd/bits/dynamic_bitset.hpp> // IWYU pragma: export; dynamic_bitset

// The sequence reading.
#include <xstd/bits/bit_sequence_adaptor.hpp> // IWYU pragma: export; bit_sequence_adaptor
#include <xstd/bits/bit_array.hpp>            // IWYU pragma: export; bit_array
#include <xstd/bits/bit_inplace_vector.hpp>   // IWYU pragma: export; bit_inplace_vector
#include <xstd/bits/bit_vector.hpp>           // IWYU pragma: export; bit_vector
#include <xstd/bits/bit_span.hpp>             // IWYU pragma: export; bit_span
#include <xstd/bits/bit_subspan.hpp>          // IWYU pragma: export; bit_subspan

// The set reading.
#include <xstd/bits/bit_set_adaptor.hpp> // IWYU pragma: export; bit_set_adaptor
#include <xstd/bits/bit_static_set.hpp>  // IWYU pragma: export; bit_static_set
#include <xstd/bits/bit_inplace_set.hpp> // IWYU pragma: export; bit_inplace_set
#include <xstd/bits/bit_set.hpp>         // IWYU pragma: export; bit_set
#include <xstd/bits/bit_set_view.hpp>    // IWYU pragma: export; bit_set_view

#endif // XSTD_BITS_HPP
