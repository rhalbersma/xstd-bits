//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_ALGORITHM_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_ALGORITHM_HPP

#include <xstd/bits/detail/intrinsic.hpp> // countr_zero
#include <xstd/bits/detail/ownership.hpp> // reads, sequence_reading_tag, set_reading_tag, storage_access
#include <algorithm>                      // min
#include <concepts>                       // same_as
#include <cstddef>                        // size_t
#include <functional>                     // greater
#include <iterator>                       // indirectly_writable, iter_reference_t
#include <ranges>                         // size
#include <type_traits>                    // remove_cvref_t
#include <utility>                        // declval

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

// Where position zero of a sequence sits in its storage: a window's offset, and zero for everything else.
template<reads_bools R>
[[nodiscard]] constexpr auto first_position(R const& r [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (whole_sequence<R>) {
                return 0UZ;
        } else {
                return storage_access::offset(r);
        }
}

// A set whose comparator is std::greater, so its iteration order is its positions from the top down.
template<class S>
concept descends = std::same_as<typename std::remove_cvref_t<S>::key_compare, std::greater<typename std::remove_cvref_t<S>::key_type>> or std::same_as<typename std::remove_cvref_t<S>::key_compare, std::greater<>>;

// Two sets whose keys mean the same positions in the same order, whatever blocks and storage hold them.
template<class S1, class S2>
concept same_keys =
        reads_keys<S1> and reads_keys<S2> and
        std::same_as<typename std::remove_cvref_t<S1>::key_type, typename std::remove_cvref_t<S2>::key_type> and
        std::same_as<typename std::remove_cvref_t<S1>::key_mapping_type, typename std::remove_cvref_t<S2>::key_mapping_type> and
        std::same_as<typename std::remove_cvref_t<S1>::key_compare, typename std::remove_cvref_t<S2>::key_compare>;

// The width a type fixes, or dynamic_extent where it is a run-time value: a window's own, else its storage's.
template<class R>
[[nodiscard]] consteval auto static_width() noexcept
        -> std::size_t
{
        if constexpr (requires { std::remove_cvref_t<R>::static_extent; }) {
                return std::remove_cvref_t<R>::static_extent;
        } else {
                return std::remove_cvref_t<decltype(storage_access::bits(std::declval<R const&>()))>::extent;
        }
}

// Two of ours of one reading that bit_hash keeps apart only where their values differ: one width kind, one width.
template<class R1, class R2>
concept comparable_readings = ((reads_bools<R1> and reads_bools<R2>) or same_keys<R1, R2>) and static_width<R1>() == static_width<R2>();

// An iterator into one of our sets: its proxy names the adaptor, and the adaptor names it back.
template<class I>
concept key_iterator =
        requires { typename std::iter_reference_t<I>::adaptor_type; } and
        reads<typename std::iter_reference_t<I>::adaptor_type, set_reading_tag> and
        std::same_as<I, typename std::iter_reference_t<I>::adaptor_type::iterator>;

// An iterator into one of our sequences that writes: its proxy names the adaptor and the iterator it came from.
template<class I>
concept bool_output_iterator =
        requires { typename std::iter_reference_t<I>::adaptor_type; } and
        reads<typename std::iter_reference_t<I>::adaptor_type, sequence_reading_tag> and
        std::same_as<I, typename std::iter_reference_t<I>::iterator> and std::indirectly_writable<I, bool>;

// The storage under one of our sequences, as the algorithms that write it reach it.
template<class R>
using bits_t = std::remove_cvref_t<decltype(storage_access::bits(std::declval<R&>()))>;

// A sequence whose bools the blit can write: its storage reached as writable, a window's or a whole sequence's.
template<class R>
concept writable_bools = reads_bools<R> and requires (R& r, bits_t<R>::block_type block) { storage_access::bits(r).block_at(0UZ, block, block); };

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

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_ALGORITHM_HPP
