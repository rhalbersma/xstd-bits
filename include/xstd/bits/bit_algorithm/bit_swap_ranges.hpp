//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // first_position, writable_bools
#include <xstd/bits/bit_algorithm/detail/blit.hpp>      // swap_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <algorithm>                                    // min, swap_ranges_result
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // begin, borrowed_iterator_t, size

namespace xstd {

// std::ranges::swap_ranges(r1, r2), a block at a time, as far as the shorter goes.
template<bits::detail::writable_bools R1, bits::detail::writable_bools R2>
constexpr auto bit_swap_ranges(R1&& r1, R2&& r2) noexcept
        -> std::ranges::swap_ranges_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
{
        using bits::detail::storage_access;
        auto const n = std::ranges::min(std::ranges::size(r1), std::ranges::size(r2));
        bits::detail::swap_bools(storage_access::bits(r1), bits::detail::first_position(r1), storage_access::bits(r2), bits::detail::first_position(r2), n);
        auto const d = static_cast<std::ptrdiff_t>(n);
        return {std::ranges::borrowed_iterator_t<R1>(std::ranges::begin(r1) + d), std::ranges::borrowed_iterator_t<R2>(std::ranges::begin(r2) + d)};
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_SWAP_RANGES_HPP
