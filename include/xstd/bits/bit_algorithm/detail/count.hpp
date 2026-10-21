//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_COUNT_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_COUNT_HPP

#include <xstd/bits/bit_algorithm/detail/sequence.hpp> // whole_sequence
#include <xstd/bits/detail/ownership.hpp>              // reads_bools, storage_access
#include <cstddef>                                     // size_t
#include <ranges>                                      // size

namespace xstd::bits::detail {

// The storage's own count on a whole sequence, the window's blocks on a subspan.
template<reads_bools R>
[[nodiscard]] constexpr auto count_true(R const& r) noexcept
        -> std::size_t
{
        if constexpr (whole_sequence<R>) {
                return storage_access::bits(r).count();
        } else {
                return storage_access::bits(r).count(storage_access::offset(r), std::ranges::size(r));
        }
}

template<reads_bools R>
[[nodiscard]] constexpr auto any_true(R const& r) noexcept
        -> bool
{
        if constexpr (whole_sequence<R>) {
                return storage_access::bits(r).any();
        } else {
                return storage_access::bits(r).any(storage_access::offset(r), std::ranges::size(r));
        }
}

// Its own helper rather than not any_true(r), so a storage spelling none() is asked in its blocks.
template<reads_bools R>
[[nodiscard]] constexpr auto none_true(R const& r) noexcept
        -> bool
{
        if constexpr (whole_sequence<R>) {
                return storage_access::bits(r).none();
        } else {
                return not storage_access::bits(r).any(storage_access::offset(r), std::ranges::size(r));
        }
}

// Not count_true(r) == size: a clear position ends it, which is what a block that is not all ones says in one test.
template<reads_bools R>
[[nodiscard]] constexpr auto all_true(R const& r) noexcept
        -> bool
{
        if constexpr (whole_sequence<R>) {
                return storage_access::bits(r).all();
        } else {
                return storage_access::bits(r).all(storage_access::offset(r), std::ranges::size(r));
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_COUNT_HPP
