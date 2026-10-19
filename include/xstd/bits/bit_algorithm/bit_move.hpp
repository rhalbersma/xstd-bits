//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bool_output_iterator, first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/blit.hpp>      // copy_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <algorithm>                                    // move_result
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::move(r, out), which for a bool is std::ranges::copy: a block of the destination at a time.
template<bits::detail::reads_bools R, bits::detail::bool_output_iterator O>
constexpr auto bit_move(R&& r, O out) noexcept
        -> std::ranges::move_result<std::ranges::borrowed_iterator_t<R>, O>
{
        using bits::detail::storage_access;
        auto const n = std::ranges::size(r);
        bits::detail::copy_bools(storage_access::iterated_bits(out), storage_access::position(out), storage_access::bits(r), bits::detail::first_position(r), n);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), out + static_cast<std::ptrdiff_t>(n)};
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP
