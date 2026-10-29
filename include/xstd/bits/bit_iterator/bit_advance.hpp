//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP
#define XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP

#include <xstd/bits/bit_iterator/detail/concepts.hpp> // bit_iterator
#include <xstd/bits/bit_iterator/detail/select.hpp>   // advance_keys, first_key_position
#include <xstd/bits/detail/ownership.hpp>             // storage_access
#include <cassert>                                    // assert
#include <iterator>                                   // iter_difference_t, random_access_iterator, ranges::advance

namespace xstd::ranges {

// std::ranges::advance(it, n): an offset step on a sequence, and on a set whole blocks skipped by their popcount.
template<bits::detail::bit_iterator I>
constexpr auto bit_advance(I& it, std::iter_difference_t<I> n) noexcept
        -> void
{
        if constexpr (std::random_access_iterator<I>) {
                std::ranges::advance(it, n);
        } else {
                // The end bounds a forward walk, the first key a backward one; stopping short means n is out of range.
                auto const bound                 = 0 < n ? bits::detail::storage_access::iterated_bits(it).size() : bits::detail::first_key_position(it);
                [[maybe_unused]] auto const rest = bits::detail::advance_keys(it, n, bound);
                assert(rest == 0);
        }
}

// std::ranges::advance(it, n, bound): no further than bound, answering the steps not taken, signed as n is.
template<bits::detail::bit_iterator I>
constexpr auto bit_advance(I& it, std::iter_difference_t<I> n, I bound) noexcept
        -> std::iter_difference_t<I>
{
        if constexpr (std::random_access_iterator<I>) {
                return std::ranges::advance(it, n, bound);
        } else {
                return bits::detail::advance_keys(it, n, bits::detail::storage_access::position(bound));
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ITERATOR_BIT_ADVANCE_HPP
