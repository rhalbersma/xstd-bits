//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_VIEW_HPP
#define XSTD_BITS_BIT_SET_VIEW_HPP

#include <xstd/bits/bit_traits.hpp>  // bit_storage, bit_traits
#include <xstd/bits/ownership.hpp>   // ownership
#include <xstd/bits/set_adaptor.hpp> // set_adaptor
#include <type_traits>               // remove_const_t

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by. [design.md#the-views-are-the-adaptors]
namespace xstd {

// An alias, as bit_subspan always was: the referring adaptor is the view, so there is nothing for a class of its
// own to add. Being the adaptor rather than deriving from it is what removes the restatements a derived class
// needed -- its own constructors, its own deduction guides, and its own enable_view, enable_borrowed_range,
// std::hash and is_range specializations, none of which a base's can serve because a derived class is not its
// base to a partial specialization. [design.md#the-views-are-the-adaptors]
template<class Bits, bit_storage<std::remove_const_t<Bits>> Traits = bit_traits<std::remove_const_t<Bits>>>
using bit_set_view = set_adaptor<Bits, ownership::refers, Traits>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SET_VIEW_HPP
