//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SUBSPAN_HPP
#define XSTD_BITS_BIT_SUBSPAN_HPP

#include <xstd/bits/basic_bit_sequence.hpp> // basic_bit_sequence
#include <xstd/bits/bit_traits.hpp>         // bit_storage, bit_traits
#include <xstd/bits/ownership.hpp>          // ownership
#include <type_traits>                      // remove_const_t

namespace xstd {

// A window on the sequence reading: what first, last and subspan return on a bit_span or on another window, and never deduced, so an alias suffices. [design.md#windows]
template<class Bits, bit_storage<std::remove_const_t<Bits>> Traits = bit_traits<std::remove_const_t<Bits>>>
using bit_subspan = basic_bit_sequence<Bits, ownership::refers, true, Traits>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SUBSPAN_HPP
