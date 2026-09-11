//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The umbrella over every container; not the ext adaptors, which would put Boost on every consumer path.
#include <xstd/bits/bit_array.hpp>          // IWYU pragma: export; bit_array
#include <xstd/bits/bit_inplace_set.hpp>    // IWYU pragma: export; bit_inplace_set
#include <xstd/bits/bit_inplace_vector.hpp> // IWYU pragma: export; bit_inplace_vector
#include <xstd/bits/bit_set.hpp>            // IWYU pragma: export; bit_set
#include <xstd/bits/bit_set_view.hpp>       // IWYU pragma: export; bit_set_view
#include <xstd/bits/bit_span.hpp>           // IWYU pragma: export; bit_span
#include <xstd/bits/bit_static_set.hpp>     // IWYU pragma: export; bit_static_set
#include <xstd/bits/bit_subspan.hpp>        // IWYU pragma: export; bit_subspan
#include <xstd/bits/bit_traits.hpp>         // IWYU pragma: export; bit_traits, bit_storage, block_readable, static_bit_extent
#include <xstd/bits/bit_vector.hpp>         // IWYU pragma: export; bit_vector
#include <xstd/bits/bitset.hpp>             // IWYU pragma: export; bitset
#include <xstd/bits/bitset_adaptor.hpp>     // IWYU pragma: export; bitset_adaptor, has_bitops
#include <xstd/bits/dynamic_bitset.hpp>     // IWYU pragma: export; dynamic_bitset
#include <xstd/bits/inplace_bitset.hpp>     // IWYU pragma: export; inplace_bitset
#include <xstd/bits/ownership.hpp>          // IWYU pragma: export; ownership
#include <xstd/bits/sequence_adaptor.hpp>   // IWYU pragma: export; sequence_adaptor
#include <xstd/bits/set_adaptor.hpp>        // IWYU pragma: export; set_adaptor

#endif // XSTD_BITS_HPP
