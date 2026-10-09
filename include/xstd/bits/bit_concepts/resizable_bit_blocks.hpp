//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_RESIZABLE_BIT_BLOCKS_HPP
#define XSTD_BITS_BIT_CONCEPTS_RESIZABLE_BIT_BLOCKS_HPP

#include <xstd/bits/bit_concepts/owned_bit_blocks.hpp> // owned_bit_blocks
#include <concepts>                                    // convertible_to
#include <ranges>                                      // end, range, range_size_t, range_value_t

namespace xstd {

// Owned blocks whose count changes at run time: what an owner of a run-time width grows and shrinks.
template<class Bits>
concept resizable_bit_blocks =
        owned_bit_blocks<Bits> and std::ranges::range<Bits> and
        requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n, std::ranges::range_value_t<Bits> const* blocks) {
                bits.resize(n, *blocks);
                bits.push_back(*blocks);
                bits.insert(std::ranges::end(bits), blocks, blocks);
                bits.clear();
                { cbits.max_size() } -> std::convertible_to<std::ranges::range_size_t<Bits>>;
        };

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_RESIZABLE_BIT_BLOCKS_HPP
