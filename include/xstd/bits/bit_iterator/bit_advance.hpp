//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP
#define XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // key_iterator
#include <xstd/bits/bit_iterator/detail/select.hpp>     // advance_keys, first_key_position
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <cassert>                                      // assert
#include <iterator>                                     // iter_difference_t

namespace xstd {

// std::ranges::advance(it, n) over a set's keys: whole blocks skipped by their popcount, the last one selected in.
template<bits::detail::key_iterator I>
constexpr auto bit_advance(I& it, std::iter_difference_t<I> n) noexcept
        -> void
{
        // Forward the end bounds the walk, backward the first key; either, reached early, is a broken precondition.
        auto const bound                 = 0 < n ? bits::detail::storage_access::iterated_bits(it).size() : bits::detail::first_key_position(it);
        [[maybe_unused]] auto const rest = bits::detail::advance_keys(it, n, bound);
        assert(rest == 0);
}

// std::ranges::advance(it, n, bound): no further than bound, answering the steps not taken, signed as n is.
template<bits::detail::key_iterator I>
constexpr auto bit_advance(I& it, std::iter_difference_t<I> n, I bound) noexcept
        -> std::iter_difference_t<I>
{
        return bits::detail::advance_keys(it, n, bits::detail::storage_access::position(bound));
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP
