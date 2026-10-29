//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_HPP

// [comparisons], comparisons
#include <xstd/bits/bit_functional/bit_equal_to.hpp>      // IWYU pragma: export; bit_equal_to
#include <xstd/bits/bit_functional/bit_not_equal_to.hpp>  // IWYU pragma: export; bit_not_equal_to
#include <xstd/bits/bit_functional/bit_greater.hpp>       // IWYU pragma: export; bit_greater
#include <xstd/bits/bit_functional/bit_less.hpp>          // IWYU pragma: export; bit_less
#include <xstd/bits/bit_functional/bit_greater_equal.hpp> // IWYU pragma: export; bit_greater_equal
#include <xstd/bits/bit_functional/bit_less_equal.hpp>    // IWYU pragma: export; bit_less_equal

// [comparisons.three.way], class compare_three_way
#include <xstd/bits/bit_functional/bit_compare_three_way.hpp> // IWYU pragma: export; bit_compare_three_way

// [unord.hash], class template hash, beside it the digest of the same bits
#include <xstd/bits/bit_functional/bit_hash.hpp>   // IWYU pragma: export; bit_hash, bit_hash_append
#include <xstd/bits/bit_functional/bit_digest.hpp> // IWYU pragma: export; bit_digest

#endif // XSTD_BITS_BIT_FUNCTIONAL_HPP
