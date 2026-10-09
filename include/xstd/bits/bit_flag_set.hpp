//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_SET_HPP
#define XSTD_BITS_BIT_FLAG_SET_HPP

#include <xstd/bits/bit_fixed_set.hpp>             // basic_bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>          // bit_flag_mapping
#include <xstd/bits/bit_type_traits/bit_least.hpp> // bit_least
#include <xstd/bits/detail/flag_block.hpp>         // flag_mask, flag_width_v
#include <xstd/ints/concepts/bit_mask.hpp>         // bit_mask
#include <xstd/ints/concepts/signed_integer.hpp>   // signed_integer
#include <cstddef>                                 // size_t
#include <functional>                              // greater

// A flag type: the set of a bitmask type's one-bit values below N, which converts with the mask itself.
namespace xstd {

// The flag type for bitmask type Mask, a signed integer too: its one-bit values, highest first, converting with Mask.
template<class Mask, std::size_t N = bits::detail::flag_width_v<Mask>>
        requires (xstd::bit_mask<Mask> or xstd::signed_integer<Mask>) and bits::detail::flag_mask<Mask>
using bit_flag_set = bit_least<basic_bit_fixed_set<Mask, std::size_t, N, bit_flag_mapping<Mask, N>, std::greater<Mask>>>; // NOLINT(modernize-use-transparent-functors): a transparent comparator would admit contains(K)

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_SET_HPP
