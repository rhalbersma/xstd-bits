//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_RIGHT_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_RIGHT_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // whole_sequence
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <cassert>                                      // assert
#include <cstddef>                                      // size_t
#include <ranges>                                       // begin, borrowed_subrange_t, end, range_difference_t

namespace xstd::ranges {

// std::ranges::shift_right(r, n), a block at a time: bit i + n takes bit i, and the n before the new begin read false.
template<bits::detail::whole_sequence R>
        requires requires (R& r, std::size_t n) { bits::detail::storage_access::bits(r) <<= n; }
constexpr auto bit_shift_right(R&& r, std::ranges::range_difference_t<R> n) noexcept
        -> std::ranges::borrowed_subrange_t<R>
{
        assert(0 <= n);
        auto const first = std::ranges::begin(r);
        auto const last  = std::ranges::end(r);
        if (n == 0) {
                return {first, last};
        }
        // No effects at a shift past the end, as the standard's has none; the storage's own needs n < size().
        if (last - first <= n) {
                return {last, last};
        }
        bits::detail::storage_access::bits(r) <<= static_cast<std::size_t>(n);
        return {first + n, last};
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SHIFT_RIGHT_HPP
