//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP

#include <xstd/bits/bit_concepts/bit_index_mapping.hpp> // bit_index_mapping
#include <xstd/bits/bit_set/bit_key_mapping.hpp>        // bit_key_mapping
#include <xstd/bits/bit_set/bit_set.hpp>                // basic_bit_set
#include <xstd/bits/bit_set/detail/set_adaptor.hpp>     // key_direction
#include <xstd/bits/detail/inplace_blocks.hpp>          // inplace_blocks_for
#include <xstd/bits/detail/num_blocks.hpp>              // num_blocks_v, tiling_block
#include <xstd/ints/concepts/unsigned_integer.hpp>      // unsigned_integer
#include <cstddef>                                      // size_t
#include <functional>                                   // less

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: bounded by the type, not by the heap.
template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
        requires bits::detail::tiling_block<Block>
using basic_bit_inplace_set = basic_bit_set<Key, bits::detail::inplace_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N, KeyMapping, Compare>;

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP
