//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_COPY_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_COPY_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>     // copy_bools
#include <xstd/bits/bit_algorithm/detail/sequence.hpp> // first_position
#include <xstd/bits/bit_iterator/detail/concepts.hpp>  // bool_output_iterator
#include <xstd/bits/detail/ownership.hpp>              // reads_bools, storage_access
#include <algorithm>                                   // copy_result
#include <cstddef>                                     // ptrdiff_t
#include <ranges>                                      // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::copy(r, out), a block of the destination at a time at any two offsets, lowest first.
template<bits::detail::reads_bools R, bits::detail::bool_output_iterator O>
constexpr auto bit_copy(R&& r, O out) noexcept
        -> std::ranges::copy_result<std::ranges::borrowed_iterator_t<R>, O>
{
        using bits::detail::storage_access;
        auto const n = std::ranges::size(r);
        bits::detail::copy_bools(storage_access::iterated_bits(out), storage_access::position(out), storage_access::bits(r), bits::detail::first_position(r), n);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), out + static_cast<std::ptrdiff_t>(n)};
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_COPY_HPP
