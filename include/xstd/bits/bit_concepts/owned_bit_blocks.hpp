//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_OWNED_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_CONCEPTS_OWNED_BIT_BLOCKS_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp>       // bit_block
#include <xstd/bits/bit_concepts/bit_blocks.hpp>      // bit_blocks
#include <xstd/bits/detail/range_const_reference.hpp> // range_const_reference_t
#include <concepts>                                   // regular, same_as
#include <ranges>                                     // range_reference_t, range_size_t

namespace xstd {

// Bit storage a container can own: a value compared by its blocks, and read-only through a const object.
template<class Bits>
concept owned_bit_blocks =
        bit_blocks<Bits> and std::regular<Bits> and
        (bit_block<Bits> or
         requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n) {
                 { bits[n] } -> std::same_as<std::ranges::range_reference_t<Bits>>;
                 // P2278R4's alias: a storage whose const subscript yields a writable reference is refused.
                 { cbits[n] } -> std::same_as<bits::detail::range_const_reference_t<Bits>>;
         });

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_OWNED_BIT_BLOCKS_HPP
