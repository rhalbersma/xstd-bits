//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_DETAIL_CONCEPTS_HPP
#define XSTD_BITS_BIT_ITERATOR_DETAIL_CONCEPTS_HPP

#include <xstd/bits/detail/ownership.hpp> // reads, sequence_reading_tag, set_reading_tag
#include <concepts>                       // same_as
#include <iterator>                       // indirectly_writable, iter_reference_t

namespace xstd::bits::detail {

// An iterator into one of our sets: its proxy names the adaptor, and the adaptor names it back.
template<class I>
concept key_iterator =
        requires { typename std::iter_reference_t<I>::adaptor_type; } and
        reads<typename std::iter_reference_t<I>::adaptor_type, set_reading_tag> and
        std::same_as<I, typename std::iter_reference_t<I>::adaptor_type::iterator>;

// An iterator into one of our sequences: its proxy names the adaptor and the iterator it came from.
template<class I>
concept bool_iterator =
        requires { typename std::iter_reference_t<I>::adaptor_type; } and
        reads<typename std::iter_reference_t<I>::adaptor_type, sequence_reading_tag> and
        std::same_as<I, typename std::iter_reference_t<I>::iterator>;

// An iterator into one of our sequences that writes.
template<class I>
concept bool_output_iterator = bool_iterator<I> and std::indirectly_writable<I, bool>;

// An iterator into one of ours, stepping over a sequence's bools or a set's keys.
template<class I>
concept bit_iterator = bool_iterator<I> or key_iterator<I>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ITERATOR_DETAIL_CONCEPTS_HPP
