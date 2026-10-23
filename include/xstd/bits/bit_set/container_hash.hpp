//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_CONTAINER_HASH_HPP
#define XSTD_BITS_BIT_SET_CONTAINER_HASH_HPP

#include <xstd/bits/bit_set/bit_fixed_set.hpp>    // basic_bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>          // basic_bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>     // bit_set_view
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/container_hash/is_tuple_like.hpp> // is_tuple_like
#include <cstddef>                                // size_t
#include <type_traits>                            // false_type

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Key, class Blocks, std::size_t N, class KeyMapping, class Compare>
struct is_range<xstd::basic_bit_set<Key, Blocks, N, KeyMapping, Compare>> : std::false_type
{};

template<class Key, class Blocks, std::size_t N, class KeyMapping, class Compare>
struct is_tuple_like<xstd::basic_bit_set<Key, Blocks, N, KeyMapping, Compare>> : std::false_type
{};

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_range<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_tuple_like<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

template<class Blocks, std::size_t N>
struct is_range<xstd::bit_set_view<Blocks, N>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_SET_CONTAINER_HASH_HPP
