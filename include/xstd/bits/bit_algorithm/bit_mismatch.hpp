//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_MISMATCH_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_MISMATCH_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp>            // mismatch_bools
#include <xstd/bits/bit_algorithm/detail/iterator_window.hpp> // bool_sentinel_for, iterator_window, rebased
#include <xstd/bits/detail/first_mismatch.hpp>                // first_mismatch
#include <xstd/bits/detail/iterator_concepts.hpp>             // bool_iterator
#include <xstd/bits/detail/ownership.hpp>                     // reads_bools, storage_access
#include <xstd/bits/detail/sequence_storage.hpp>              // first_position, whole_sequence
#include <algorithm>                                          // min, mismatch_result
#include <concepts>                                           // same_as
#include <cstddef>                                            // ptrdiff_t, size_t
#include <ranges>                                             // begin, borrowed_iterator_t, size
#include <type_traits>                                        // remove_cvref_t
#include <utility>                                            // pair

namespace xstd::ranges {

// std::ranges::mismatch(r1, r2), a block at a time as far as the shorter goes, whatever the two blocks and offsets.
template<bits::detail::reads_bools R1, bits::detail::reads_bools R2>
[[nodiscard]] constexpr auto bit_mismatch(R1&& r1, R2&& r2) noexcept
        -> std::ranges::mismatch_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
{
        using bits::detail::storage_access;
        auto const n1 = std::ranges::size(r1);
        auto const n2 = std::ranges::size(r2);
        auto const n  = [&] -> std::size_t {
                if constexpr (bits::detail::whole_sequence<R1> and std::same_as<std::remove_cvref_t<R1>, std::remove_cvref_t<R2>> and requires { storage_access::bits(r1).first_difference(storage_access::bits(r2)); }) {
                        return n1 <= n2 ? bits::detail::first_mismatch(storage_access::bits(r1), storage_access::bits(r2), n1) : bits::detail::first_mismatch(storage_access::bits(r2), storage_access::bits(r1), n2);
                } else {
                        return bits::detail::mismatch_bools(storage_access::bits(r1), bits::detail::first_position(r1), storage_access::bits(r2), bits::detail::first_position(r2), std::ranges::min(n1, n2));
                }
        }();
        auto const d = static_cast<std::ptrdiff_t>(n);
        return {std::ranges::borrowed_iterator_t<R1>(std::ranges::begin(r1) + d), std::ranges::borrowed_iterator_t<R2>(std::ranges::begin(r2) + d)};
}

// std::ranges::mismatch(first1, last1, first2, last2), over both pairs' positions as windows.
template<bits::detail::bool_iterator I1, bits::detail::bool_sentinel_for<I1> S1, bits::detail::bool_iterator I2, bits::detail::bool_sentinel_for<I2> S2>
[[nodiscard]] constexpr auto bit_mismatch(I1 first1, S1 last1, I2 first2, S2 last2) noexcept
        -> std::ranges::mismatch_result<I1, I2>
{
        auto w1               = bits::detail::iterator_window(first1, last1);
        auto w2               = bits::detail::iterator_window(first2, last2);
        auto const [in1, in2] = xstd::ranges::bit_mismatch(w1, w2);
        return {bits::detail::rebased(first1, in1), bits::detail::rebased(first2, in2)};
}

} // namespace xstd::ranges

namespace xstd {

// std::mismatch(first1, last1, first2): the second range as long as the first.
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
[[nodiscard]] constexpr auto bit_mismatch(I1 first1, I1 last1, I2 first2) noexcept
        -> std::pair<I1, I2>
{
        auto const [in1, in2] = ranges::bit_mismatch(first1, last1, first2, first2 + (last1 - first1));
        return {in1, in2};
}

// std::mismatch(first1, last1, first2, last2).
template<bits::detail::bool_iterator I1, bits::detail::bool_iterator I2>
[[nodiscard]] constexpr auto bit_mismatch(I1 first1, I1 last1, I2 first2, I2 last2) noexcept
        -> std::pair<I1, I2>
{
        auto const [in1, in2] = ranges::bit_mismatch(first1, last1, first2, last2);
        return {in1, in2};
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_MISMATCH_HPP
