//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_HPP
#define XSTD_BITS_ALGORITHM_HPP

// The sequence reading's algorithms, std::ranges' over bools a block at a time.
#include <xstd/bits/algorithm/bit_all_of.hpp>   // IWYU pragma: export; bit_all_of
#include <xstd/bits/algorithm/bit_any_of.hpp>   // IWYU pragma: export; bit_any_of
#include <xstd/bits/algorithm/bit_none_of.hpp>  // IWYU pragma: export; bit_none_of
#include <xstd/bits/algorithm/bit_count.hpp>    // IWYU pragma: export; bit_count
#include <xstd/bits/algorithm/bit_mismatch.hpp> // IWYU pragma: export; bit_mismatch
#include <xstd/bits/algorithm/bit_reverse.hpp>  // IWYU pragma: export; bit_reverse
#include <xstd/bits/algorithm/bit_rotate.hpp>   // IWYU pragma: export; bit_rotate

// The set reading's, over keys.
#include <xstd/bits/algorithm/bit_includes.hpp> // IWYU pragma: export; bit_includes
#include <xstd/bits/algorithm/bit_disjoint.hpp> // IWYU pragma: export; bit_disjoint

#endif // XSTD_BITS_ALGORITHM_HPP
