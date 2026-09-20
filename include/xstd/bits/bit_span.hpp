//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_HPP
#define XSTD_BITS_BIT_SPAN_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/ownership.hpp>                       // ownership
#include <xstd/bits/sequence_adaptor.hpp>                // sequence_adaptor
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN

// The sequence reading over bits it does not own: like std::span it neither compares nor orders.
namespace xstd {

// An alias differing from bit_subspan in one non-type argument: this is the whole sequence, that one a window.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits>
using bit_span = sequence_adaptor<Bits, ownership::refers, false>;

} // namespace xstd

#endif // XSTD_BITS_BIT_SPAN_HPP
