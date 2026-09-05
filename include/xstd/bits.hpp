//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_HPP
#define XSTD_BITS_HPP

// The front door over every container; not the ext adaptors, which would put Boost on every consumer path.
#include <xstd/bits/bit_array.hpp>      // IWYU pragma: export; bit_array
#include <xstd/bits/bit_proxy.hpp>      // IWYU pragma: export; bit_set_iterator, bit_set_reference, bit_sequence_iterator, bit_sequence_reference
#include <xstd/bits/bit_static_set.hpp> // IWYU pragma: export; bit_static_set
#include <xstd/bits/bitset.hpp>         // IWYU pragma: export; bitset
#include <xstd/bits/block_sequence.hpp> // IWYU pragma: export; block_sequence, block_array, block_vector
#include <xstd/bits/ranges.hpp>         // IWYU pragma: export; set_view, sequence_view

#endif // XSTD_BITS_HPP
