//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_RANGES_SET_VIEW_HPP
#define XSTD_BITS_RANGES_SET_VIEW_HPP

#include <xstd/bits/basic_bit_set.hpp> // basic_bit_set
#include <xstd/bits/bit_traits.hpp>    // bit_storage, bit_traits
#include <xstd/bits/ownership.hpp>     // ownership
#include <type_traits>                 // remove_const_t

// The set reading over bits it does not own: the referring adaptor, under the name the sieve calls it by. [design.md#the-views-are-the-adaptors]
namespace xstd::ranges {

template<class Bits, bit_storage<std::remove_const_t<Bits>> Traits = bit_traits<std::remove_const_t<Bits>>>
using set_view = basic_bit_set<Bits, ownership::refers, Traits>;

}       // namespace xstd::ranges

namespace xstd {

using ranges::set_view;

}       // namespace xstd

#endif  // XSTD_BITS_RANGES_SET_VIEW_HPP
