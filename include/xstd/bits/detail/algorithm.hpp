//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ALGORITHM_HPP
#define XSTD_BITS_DETAIL_ALGORITHM_HPP

#include <xstd/bits/detail/intrin.hpp>    // countr_zero
#include <xstd/bits/detail/ownership.hpp> // reads, sequence_reading_tag, set_reading_tag, storage_access
#include <algorithm>                      // min
#include <cstddef>                        // size_t
#include <ranges>                         // size
#include <type_traits>                    // remove_cvref_t

namespace xstd::bits::detail {

// A container or view of this library that reads its blocks as bools.
template<class R>
concept reads_bools = reads<std::remove_cvref_t<R>, sequence_reading_tag>;

// One that reads them as keys.
template<class R>
concept reads_keys = reads<std::remove_cvref_t<R>, set_reading_tag>;

// A sequence reading all of its storage: a subspan's window shares its end blocks with positions outside it.
template<class R>
concept whole_sequence = reads_bools<R> and not std::remove_cvref_t<R>::is_windowed;

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

// The first of the shorter's n positions where the two differ, or n: a difference in its padding lies past n.
template<class Bits>
[[nodiscard]] constexpr auto first_mismatch(Bits const& shorter, Bits const& longer, std::size_t n) noexcept
        -> std::size_t
{
        if (n == 0UZ) {
                return 0UZ;
        }
        auto const [index, diff] = shorter.first_difference(longer);
        if (diff == typename Bits::block_type{}) {
                return n;
        }
        return std::min((index * Bits::bits_per_block) + static_cast<std::size_t>(countr_zero(diff)), n);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_ALGORITHM_HPP
