//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
#define XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP

#include <xstd/bits/bit_concepts/bit_index_mapping.hpp> // bit_index_mapping
#include <xstd/bits/bit_set/bit_key_mapping.hpp>        // bit_key_mapping
#include <xstd/bits/bit_set/bit_set.hpp>                // basic_bit_set
#include <xstd/bits/bit_set/container_hash.hpp>         // IWYU pragma: export; is_range, is_tuple_like
#include <xstd/bits/bit_set/detail/set_adaptor.hpp>     // key_direction
#include <xstd/bits/bit_set/hash.hpp>                   // IWYU pragma: export; hash
#include <xstd/bits/detail/num_blocks.hpp>              // num_blocks_v, tiling_block
#include <xstd/bits/ext/boost/detail/small_blocks.hpp>  // bit_storage_rebind
#include <xstd/ints/concepts/unsigned_integer.hpp>      // unsigned_integer
#include <boost/container/new_allocator.hpp>            // new_allocator
#include <boost/container/small_vector.hpp>             // small_vector
#include <cstddef>                                      // size_t
#include <functional>                                   // less
#include <span>                                         // dynamic_extent

namespace xstd {

// The set reading over the small-vector column: the blocks of N bits inline, the heap past them, Boost's allocator.
template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>, class Alloc = boost::container::new_allocator<Block>>
        requires bits::detail::tiling_block<Block>
using basic_bit_small_set = basic_bit_set<Key, boost::container::small_vector<Block, bits::detail::num_blocks_v<Block, N>, Alloc>, std::dynamic_extent, KeyMapping, Compare>;

template<std::size_t N>
using bit_small_set = basic_bit_small_set<std::size_t, std::size_t, N>;

} // namespace xstd

#endif // XSTD_BITS_EXT_BOOST_BIT_SMALL_SET_HPP
