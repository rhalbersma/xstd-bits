//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_HPP

// [array], class template array
#include <xstd/bits/bit_array/bit_array.hpp> // IWYU pragma: export; basic_bit_array, bit_array
// [array.creation], array creation functions
#include <xstd/bits/bit_array/to_array.hpp> // IWYU pragma: export; to_bit_array
// [array.tuple], tuple interface
#include <xstd/bits/bit_array/tuple_element.hpp> // IWYU pragma: export; tuple_element, tuple_size
// [unord.hash], beside it: std::hash, which <array> does not specialize for std::array
#include <xstd/bits/bit_array/hash.hpp> // IWYU pragma: export; hash
// Boost.ContainerHash's traits, https://www.boost.org/libs/container_hash
#include <xstd/bits/bit_array/container_hash.hpp> // IWYU pragma: export; is_range, is_tuple_like

#endif // XSTD_BITS_BIT_ARRAY_HPP
