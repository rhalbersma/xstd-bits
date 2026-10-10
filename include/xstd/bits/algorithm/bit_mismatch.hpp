//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_ALGORITHM_BIT_MISMATCH_HPP
#define XSTD_BITS_ALGORITHM_BIT_MISMATCH_HPP

#include <xstd/bits/detail/algorithm.hpp> // first_mismatch, reads_bools, storage_access, whole_sequence
#include <algorithm>                      // mismatch, mismatch_result
#include <concepts>                       // same_as
#include <cstddef>                        // ptrdiff_t
#include <ranges>                         // begin, borrowed_iterator_t, size
#include <type_traits>                    // remove_cvref_t

namespace xstd {

// std::ranges::mismatch(r1, r2), a block at a time over two whole sequences; a window is compared a bool at a time.
template<class R1, class R2>
        requires bits::detail::reads_bools<R1> and std::same_as<std::remove_cvref_t<R1>, std::remove_cvref_t<R2>>
[[nodiscard]] constexpr auto bit_mismatch(R1&& r1, R2&& r2) noexcept
        -> std::ranges::mismatch_result<std::ranges::borrowed_iterator_t<R1>, std::ranges::borrowed_iterator_t<R2>>
{
        using bits::detail::storage_access;
        if constexpr (bits::detail::whole_sequence<R1> and requires { storage_access::bits(r1).first_difference(storage_access::bits(r2)); }) {
                auto const n1 = std::ranges::size(r1);
                auto const n2 = std::ranges::size(r2);
                auto const n  = n1 <= n2 ? bits::detail::first_mismatch(storage_access::bits(r1), storage_access::bits(r2), n1) : bits::detail::first_mismatch(storage_access::bits(r2), storage_access::bits(r1), n2);
                auto const d  = static_cast<std::ptrdiff_t>(n);
                return {std::ranges::borrowed_iterator_t<R1>(std::ranges::begin(r1) + d), std::ranges::borrowed_iterator_t<R2>(std::ranges::begin(r2) + d)};
        } else {
                auto const [in1, in2] = std::ranges::mismatch(r1, r2);
                return {std::ranges::borrowed_iterator_t<R1>(in1), std::ranges::borrowed_iterator_t<R2>(in2)};
        }
}

} // namespace xstd

#endif // XSTD_BITS_ALGORITHM_BIT_MISMATCH_HPP
