//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bool_output_iterator, first_position, reads_bools
#include <xstd/bits/bit_algorithm/detail/blit.hpp>      // copy_bools
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <algorithm>                                    // copy_backward_result
#include <cstddef>                                      // ptrdiff_t
#include <ranges>                                       // borrowed_iterator_t, end, size

namespace xstd::ranges {

// std::ranges::copy_backward(r, out_last), a block at a time from the top, so it may overlap r from above.
template<bits::detail::reads_bools R, bits::detail::bool_output_iterator O>
constexpr auto bit_copy_backward(R&& r, O out_last) noexcept
        -> std::ranges::copy_backward_result<std::ranges::borrowed_iterator_t<R>, O>
{
        using bits::detail::storage_access;
        auto const n     = std::ranges::size(r);
        auto const first = out_last - static_cast<std::ptrdiff_t>(n);
        bits::detail::copy_bools<true>(storage_access::iterated_bits(first), storage_access::position(first), storage_access::bits(r), bits::detail::first_position(r), n);
        return {std::ranges::borrowed_iterator_t<R>(std::ranges::end(r)), first};
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP
