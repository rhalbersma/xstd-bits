//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARE_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARE_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // bits_t, descends, first_mismatch, first_position, reads_keys, static_width, whole_sequence
#include <xstd/bits/detail/comparisons.hpp>             // numeric_three_way, set_equal, three_way
#include <xstd/bits/detail/intrin.hpp>                  // countr_zero
#include <xstd/bits/detail/ownership.hpp>               // sequence_reading_tag, set_reading_tag, storage_access
#include <xstd/bits/detail/shift.hpp>                   // partial_block_mask
#include <algorithm>                                    // max, min
#include <cassert>                                      // assert
#include <compare>                                      // strong_ordering
#include <concepts>                                     // same_as
#include <cstddef>                                      // size_t
#include <cstdint>                                      // uint64_t
#include <limits>                                       // numeric_limits
#include <ranges>                                       // size
#include <span>                                         // dynamic_extent
#include <type_traits>                                  // remove_cvref_t
#include <utility>                                      // declval

namespace xstd::bits::detail {

// What two storages of different block types are compared in, a word at a time.
inline constexpr auto word_digits = static_cast<std::size_t>(std::numeric_limits<std::uint64_t>::digits);

// Bits [n, n + 64) of c as one word, any at or past last reading as zero, assembled from c's own blocks.
template<class Bits>
[[nodiscard]] constexpr auto word_at(Bits const& c, std::size_t n, std::size_t last) noexcept
        -> std::uint64_t
{
        if (last <= n) {
                return 0U;
        }
        auto const count = std::ranges::min(word_digits, last - n);
        auto word        = std::uint64_t{0};
        for (auto j = 0UZ; j < count; j += Bits::bits_per_block) {
                assert(j < word_digits);
                word |= static_cast<std::uint64_t>(c.block_at(n + j)) << j;
        }
        return word & partial_block_mask<std::uint64_t>(count);
}

// Two storages one sequence or set can read without converting a block, and so compared in their own blocks.
template<class R1, class R2>
concept same_storage = std::same_as<std::remove_cvref_t<decltype(storage_access::bits(std::declval<R1 const&>()))>, std::remove_cvref_t<decltype(storage_access::bits(std::declval<R2 const&>()))>>;

// A sequence whose size two values of its type can disagree on: a run-time width over storage that can hold a bool.
template<class R>
concept varying_size = static_width<R>() == std::dynamic_extent and not bits_t<R>::has_zero_capacity;

// Whether two sequences hold the same bools: in their blocks where they can, else a word at a time.
template<class R1, class R2>
[[nodiscard]] constexpr auto equal_bools(R1 const& r1, R2 const& r2) noexcept
        -> bool
{
        auto const n = std::ranges::size(r1);
        if constexpr (varying_size<R1> or varying_size<R2>) {
                if (n != std::ranges::size(r2)) {
                        return false;
                }
        }
        auto const& x = storage_access::bits(r1);
        auto const& y = storage_access::bits(r2);
        if constexpr (same_storage<R1, R2> and whole_sequence<R1> and whole_sequence<R2>) {
                return first_mismatch(x, y, n) == n;
        } else {
                auto const xf = first_position(r1);
                auto const yf = first_position(r2);
                for (auto k = 0UZ; k < n; k += word_digits) {
                        if (word_at(x, xf + k, xf + n) != word_at(y, yf + k, yf + n)) {
                                return false;
                        }
                }
                return true;
        }
}

// Whether two sets hold the same keys, at any two widths: words past a storage's width hold none.
template<class S1, class S2>
[[nodiscard]] constexpr auto equal_keys(S1 const& s1, S2 const& s2) noexcept
        -> bool
{
        auto const& x = storage_access::bits(s1);
        auto const& y = storage_access::bits(s2);
        if constexpr (same_storage<S1, S2>) {
                return set_equal(x, y);
        } else {
                auto const n = std::ranges::max(x.size(), y.size());
                for (auto k = 0UZ; k < n; k += word_digits) {
                        if (word_at(x, k, x.size()) != word_at(y, k, y.size())) {
                                return false;
                        }
                }
                return true;
        }
}

// Lexicographic over the bools, false before true, a shorter sequence that is a prefix of the longer coming first.
template<class R1, class R2>
[[nodiscard]] constexpr auto three_way_bools(R1 const& r1, R2 const& r2) noexcept
        -> std::strong_ordering
{
        auto const& x = storage_access::bits(r1);
        auto const& y = storage_access::bits(r2);
        if constexpr (same_storage<R1, R2> and whole_sequence<R1> and whole_sequence<R2>) {
                return three_way<sequence_reading_tag>(x, y);
        } else {
                auto const n1 = std::ranges::size(r1);
                auto const n2 = std::ranges::size(r2);
                auto const n  = std::ranges::min(n1, n2);
                auto const xf = first_position(r1);
                auto const yf = first_position(r2);
                for (auto k = 0UZ; k < n; k += word_digits) {
                        auto const xw = word_at(x, xf + k, xf + n);
                        auto const yw = word_at(y, yf + k, yf + n);
                        if (xw != yw) {
                                // The first difference decides, and there one side holds true and the other false.
                                auto const first = std::uint64_t{1} << countr_zero(static_cast<std::uint64_t>(xw ^ yw));
                                return (xw & first) != 0U ? std::strong_ordering::greater : std::strong_ordering::less;
                        }
                }
                if constexpr (varying_size<R1> or varying_size<R2>) {
                        return n1 <=> n2;
                } else {
                        return std::strong_ordering::equal;
                }
        }
}

// Whether c holds a position above p, where p may lie at or past its width.
template<class Bits>
[[nodiscard]] constexpr auto holds_above(Bits const& c, std::size_t p) noexcept
        -> bool
{
        return p + 1UZ < c.size() and c.any(p + 1UZ, c.size() - p - 1UZ);
}

// Two storages' masks compared as numbers, highest word first: the order of descending keys.
template<class X, class Y>
[[nodiscard]] constexpr auto numeric_words_three_way(X const& x, Y const& y) noexcept
        -> std::strong_ordering
{
        auto const words = (std::ranges::max(x.size(), y.size()) + word_digits - 1UZ) / word_digits;
        for (auto k = words - 1UZ; k < words; --k) {
                auto const xw = word_at(x, k * word_digits, x.size());
                auto const yw = word_at(y, k * word_digits, y.size());
                if (xw != yw) {
                        return xw < yw ? std::strong_ordering::less : std::strong_ordering::greater;
                }
        }
        return std::strong_ordering::equal;
}

// Two storages' ascending keys compared lexicographically, a word at a time from the lowest.
template<class X, class Y>
[[nodiscard]] constexpr auto ascending_words_three_way(X const& x, Y const& y) noexcept
        -> std::strong_ordering
{
        auto const n = std::ranges::max(x.size(), y.size());
        for (auto k = 0UZ; k < n; k += word_digits) {
                auto const xw = word_at(x, k, x.size());
                auto const yw = word_at(y, k, y.size());
                if (xw != yw) {
                        // The lower key wins, unless its set ends there and the other goes on.
                        auto const p = k + countr_zero(static_cast<std::uint64_t>(xw ^ yw));
                        if ((xw & (std::uint64_t{1} << (p - k))) != 0U) {
                                return holds_above(y, p) ? std::strong_ordering::less : std::strong_ordering::greater;
                        }
                        return holds_above(x, p) ? std::strong_ordering::greater : std::strong_ordering::less;
                }
        }
        return std::strong_ordering::equal;
}

// std::set's order over the keys in iteration order: lexicographic ascending, the masks as numbers descending.
template<class S1, class S2>
[[nodiscard]] constexpr auto three_way_keys(S1 const& s1, S2 const& s2) noexcept
        -> std::strong_ordering
{
        auto const& x = storage_access::bits(s1);
        auto const& y = storage_access::bits(s2);
        if constexpr (same_storage<S1, S2> and descends<S1>) {
                return numeric_three_way(x, y);
        } else if constexpr (same_storage<S1, S2>) {
                return three_way<set_reading_tag>(x, y);
        } else if constexpr (descends<S1>) {
                return numeric_words_three_way(x, y);
        } else {
                return ascending_words_three_way(x, y);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_COMPARE_HPP
