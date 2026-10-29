//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HASH_HPP
#define XSTD_BITS_BIT_VECTOR_HASH_HPP

#include <xstd/bits/bit_vector/bit_vector.hpp> // basic_bit_vector
#include <cstddef>                             // size_t
#include <functional>                          // hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Blocks, std::size_t N>
struct hash<xstd::basic_bit_vector<Blocks, N>> : hash<typename xstd::basic_bit_vector<Blocks, N>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_VECTOR_HASH_HPP
