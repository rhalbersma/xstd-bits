//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SUBSPAN_HPP
#define XSTD_BITS_BIT_SUBSPAN_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // specialization_of_contiguous_bit_container
#include <xstd/bits/ownership.hpp>                       // ownership
#include <xstd/bits/sequence_adaptor.hpp>                // sequence_adaptor

namespace xstd {

// A window on the sequence reading: what first, last and subspan return on a bit_span or on another window, and never deduced, so an alias suffices. [design.md#windows]
template<detail::bits::specialization_of_contiguous_bit_container Bits>
using bit_subspan = sequence_adaptor<Bits, ownership::refers, true>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SUBSPAN_HPP
