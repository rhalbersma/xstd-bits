//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // copy_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator, bool_output_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position
#include <algorithm>                                          // copy_backward_result
#include <cstddef>                                            // ptrdiff_t
#include <ranges>                                             // borrowed_iterator_t, end, size

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

// std::ranges::copy_backward(first, last, out_last), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_output_iterator I2>
constexpr auto bit_copy_backward(I1 first, S1 last, I2 out_last) noexcept
        -> std::ranges::copy_backward_result<I1, I2>
{
        auto w                 = bits::detail::iterator_window(first, last);
        auto const [in, begin] = xstd::ranges::bit_copy_backward(w, out_last);
        return {bits::detail::rebased(first, in), begin};
}

} // namespace xstd::ranges

namespace xstd {

// std::copy_backward(first, last, out_last): the beginning of what was written.
template<bits::detail::bool_iterator I1, bits::detail::bool_output_iterator I2>
constexpr auto bit_copy_backward(I1 first, I1 last, I2 out_last) noexcept
        -> I2
{
        return ranges::bit_copy_backward(first, last, out_last).out;
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_COPY_BACKWARD_HPP
