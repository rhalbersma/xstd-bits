//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP
#define XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // descends, key_iterator
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <iterator>                                     // iter_difference_t, iter_reference_t

namespace xstd::ranges {

// std::ranges::distance(first, last) over a set's keys: the keys between them counted a block at a time.
template<bits::detail::key_iterator I>
[[nodiscard]] constexpr auto bit_distance(I first, I last) noexcept
        -> std::iter_difference_t<I>
{
        using bits::detail::storage_access;
        auto&& c         = storage_access::iterated_bits(first);
        auto const size  = c.size();
        auto const from  = storage_access::position(first);
        auto const until = storage_access::position(last);
        if constexpr (bits::detail::descends<typename std::iter_reference_t<I>::adaptor_type>) {
                // Down from first's key to just above last's, or to position zero where last is the end below it.
                if (from == size) {
                        return 0;
                }
                auto const lo = until == size ? 0UZ : until + 1UZ;
                return static_cast<std::iter_difference_t<I>>(c.count(lo, from + 1UZ - lo));
        } else {
                return static_cast<std::iter_difference_t<I>>(c.count(from, until - from));
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ITERATOR_BIT_DISTANCE_HPP
