//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SPAN_HPP
#define XSTD_BITS_BIT_SPAN_HPP

#include <xstd/bits/detail/contiguous_bit_container.hpp> // specialization_of_contiguous_bit_container
#include <xstd/bits/ownership.hpp>                       // ownership
#include <xstd/bits/sequence_adaptor.hpp>                // sequence_adaptor

// The sequence reading over bits it does not own: the referring adaptor, which like std::span neither compares nor orders. [design.md#the-views-are-the-adaptors]
namespace xstd {

// An alias, as bit_subspan always was, and differing from it in one non-type argument: this is the whole sequence, that one a window on it. [design.md#the-views-are-the-adaptors]
template<detail::bits::specialization_of_contiguous_bit_container Bits>
using bit_span = sequence_adaptor<Bits, ownership::refers, false>;

}       // namespace xstd

#endif  // XSTD_BITS_BIT_SPAN_HPP
