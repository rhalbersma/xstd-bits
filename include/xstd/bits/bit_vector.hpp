//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VECTOR_HPP
#define XSTD_BITS_BIT_VECTOR_HPP

// [vector.bool.pspc], partial class template specialization vector<bool, Allocator>
#include <xstd/bits/bit_vector/bit_vector.hpp> // IWYU pragma: export; basic_bit_vector, bit_vector
// [vector.syn], hash support
#include <xstd/bits/bit_vector/hash.hpp> // IWYU pragma: export; hash
// Boost.ContainerHash's traits, https://www.boost.org/libs/container_hash
#include <xstd/bits/bit_vector/container_hash.hpp> // IWYU pragma: export; is_range, is_tuple_like

#endif // XSTD_BITS_BIT_VECTOR_HPP
