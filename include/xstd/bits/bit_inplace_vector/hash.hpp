//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_HASH_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_HASH_HPP

#include <xstd/bits/bit_inplace_vector/bit_inplace_vector.hpp> // basic_bit_inplace_vector
#include <functional>                                          // hash
#include <cstddef>                                             // size_t

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Block, std::size_t N>
struct hash<xstd::basic_bit_inplace_vector<Block, N>> : hash<typename xstd::basic_bit_inplace_vector<Block, N>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_HASH_HPP
