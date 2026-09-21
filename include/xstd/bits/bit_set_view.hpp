//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_VIEW_HPP
#define XSTD_BITS_BIT_SET_VIEW_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/ownership.hpp>                       // storage
#include <xstd/bits/set_adaptor.hpp>                     // set_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN

// The set reading over bits it does not own: the referring adaptor under the name the sieve calls it by.
namespace xstd {

// An alias: the referring adaptor is the view, so a class of its own would add nothing.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits>
using bit_set_view = set_adaptor<Bits, storage::borrowed>;

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_VIEW_HPP
