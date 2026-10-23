//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_HASH_HPP
#define XSTD_BITS_BIT_SET_HASH_HPP

#include <xstd/bits/bit_set/bit_fixed_set.hpp> // basic_bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>       // basic_bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>  // bit_set_view
#include <cstddef>                             // size_t
#include <functional>                          // hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Key, class Blocks, std::size_t N, class KeyMapping, class Compare>
struct hash<xstd::basic_bit_set<Key, Blocks, N, KeyMapping, Compare>> : hash<typename xstd::basic_bit_set<Key, Blocks, N, KeyMapping, Compare>::adaptor_type>
{};

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct hash<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : hash<typename xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>::adaptor_type>
{};

template<class Blocks, std::size_t N>
struct hash<xstd::bit_set_view<Blocks, N>> : hash<typename xstd::bit_set_view<Blocks, N>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_SET_HASH_HPP
