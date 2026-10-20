//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_HPP

// [inplace.vector], class template inplace_vector
#include <xstd/bits/bit_inplace_vector/bit_inplace_vector.hpp> // IWYU pragma: export; basic_bit_inplace_vector, bit_inplace_vector

// [inplace.vector.erasure], erasure
#include <xstd/bits/bit_inplace_vector/erase_if.hpp> // IWYU pragma: export; erase, erase_if

// [unord.hash], beside it: std::hash, which <inplace_vector> does not specialize for std::inplace_vector
#include <xstd/bits/bit_inplace_vector/hash.hpp> // IWYU pragma: export; hash

// Boost.ContainerHash's traits, https://www.boost.org/libs/container_hash
#include <xstd/bits/bit_inplace_vector/container_hash.hpp> // IWYU pragma: export; is_range, is_tuple_like

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_HPP
