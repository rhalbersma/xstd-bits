//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_HPP
#define XSTD_BITS_EXT_BOOST_HPP

// The three readings over Boost's small_vector, with no umbrella: an adapted library is asked for by name.

// The sequence reading.
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // IWYU pragma: export; basic_bit_small_vector, bit_small_vector
#include <xstd/bits/bit_vector/erase_if.hpp>        // IWYU pragma: export; erase, erase_if

// The valarray reading.
#include <xstd/bits/ext/boost/bit_small_valarray.hpp> // IWYU pragma: export; basic_bit_small_valarray, bit_small_valarray

// The set reading.
#include <xstd/bits/ext/boost/bit_small_set.hpp> // IWYU pragma: export; basic_bit_small_set, bit_small_set
#include <xstd/bits/bit_set/erase_if.hpp>        // IWYU pragma: export; erase_if

#endif // XSTD_BITS_EXT_BOOST_HPP
