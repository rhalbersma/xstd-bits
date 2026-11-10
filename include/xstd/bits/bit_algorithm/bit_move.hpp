//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // copy_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator, bool_output_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position
#include <algorithm>                                          // move_result
#include <cstddef>                                            // ptrdiff_t
#include <ranges>                                             // borrowed_iterator_t, end, size

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

// std::ranges::move(first, last, out), over the positions [first, last) as a window.
template<bits::detail::bool_iterator I, bits::detail::bool_sentinel_for<I> S, bits::detail::bool_output_iterator O>
constexpr auto bit_move(I first, S last, O out) noexcept
        -> std::ranges::move_result<I, O>
{
        auto w               = bits::detail::iterator_window(first, last);
        auto const [in, end] = xstd::ranges::bit_move(w, out);
        return {bits::detail::rebased(first, in), end};
}

} // namespace xstd::ranges

namespace xstd {

// std::move(first, last, out): the end of what was written.
template<bits::detail::bool_iterator I, bits::detail::bool_output_iterator O>
constexpr auto bit_move(I first, I last, O out) noexcept
        -> O
{
        return xstd::ranges::bit_move(first, last, out).out;
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_MOVE_HPP
