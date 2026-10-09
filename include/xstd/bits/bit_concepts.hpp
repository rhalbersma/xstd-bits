//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_HPP
#define XSTD_BITS_BIT_CONCEPTS_HPP

// What holds the bits, each concept after the one it refines.
#include <xstd/bits/bit_concepts/bit_block.hpp>            // IWYU pragma: export; bit_block
#include <xstd/bits/bit_concepts/bit_block_range.hpp>      // IWYU pragma: export; bit_block_range
#include <xstd/bits/bit_concepts/bit_blocks.hpp>           // IWYU pragma: export; bit_blocks
#include <xstd/bits/bit_concepts/owned_bit_blocks.hpp>     // IWYU pragma: export; owned_bit_blocks
#include <xstd/bits/bit_concepts/resizable_bit_blocks.hpp> // IWYU pragma: export; resizable_bit_blocks

// What a set owner asks of the mapping that places its keys in bits.
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>       // IWYU pragma: export; bit_index_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // IWYU pragma: export; sized_bit_index_mapping
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp>        // IWYU pragma: export; bit_mask_mapping

// Blocks an owner takes as they are, and positions converted between any two widths.
#include <xstd/bits/bit_concepts/bit_constructible_from.hpp> // IWYU pragma: export; bit_constructible_from
#include <xstd/bits/bit_concepts/bit_convertible_to.hpp>     // IWYU pragma: export; bit_convertible_to

#endif // XSTD_BITS_BIT_CONCEPTS_HPP
