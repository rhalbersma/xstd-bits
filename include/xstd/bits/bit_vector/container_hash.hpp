//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_CONTAINER_HASH_HPP
#define XSTD_BITS_BIT_VECTOR_CONTAINER_HASH_HPP

#include <xstd/bits/bit_vector/bit_vector.hpp>    // basic_bit_vector
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/container_hash/is_tuple_like.hpp> // is_tuple_like
#include <type_traits>                            // false_type

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Block, class Allocator>
struct is_range<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

template<class Block, class Allocator>
struct is_tuple_like<xstd::basic_bit_vector<Block, Allocator>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_BIT_VECTOR_CONTAINER_HASH_HPP
