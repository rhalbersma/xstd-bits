//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_MASK_MAPPING_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_MASK_MAPPING_HPP

#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <concepts>                                           // same_as

namespace xstd {

// A mapping whose keys are the one-bit values of Key, so that any Key value is a set of them: a mask.
template<class Mapping, class Key>
concept bit_mask_mapping =
        sized_bit_index_mapping<Mapping, Key> and
        // Unsigned, never the signed type under an enumeration such as std::launch: a flag may sit on its sign bit.
        xstd::unsigned_integer<typename Mapping::block_type> and
        requires (Key mask, Mapping::block_type block) {
                { Mapping::to_block(mask) } -> std::same_as<typename Mapping::block_type>; // every bit of mask, at its position
                { Mapping::from_block(block) } -> std::same_as<Key>;                       // the mask with those bits
        };

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_MASK_MAPPING_HPP
