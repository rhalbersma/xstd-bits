//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_COMPARISONS_HPP
#define XSTD_BITS_DETAIL_COMPARISONS_HPP

#include <xstd/bits/detail/bit_container.hpp> // bit_container_type
#include <xstd/bits/detail/intrin.hpp>        // countr_zero
#include <xstd/bits/detail/pred.hpp>          // intersects
#include <xstd/bits/detail/shift.hpp>         // shl
#include <algorithm>                          // equal, max, min
#include <compare>                            // strong_ordering
#include <cstddef>                            // ptrdiff_t, size_t
#include <ranges>                             // begin

// Each reading's equality and ordering over a bit_container, spelled in its primitives and named for the reading.
namespace xstd::bits::detail {

// The set reading's equality, where width is capacity: neither value being the subject, it takes both.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto set_equal(Bits const& x, Bits const& y) noexcept
        -> bool
{
        if constexpr (Bits::has_static_size) {
                // One width, so holding the same positions and being equal are the same statement.
                return x == y;
        } else {
                // ranges::equal over the shared prefix as an iterator pair, which lowers to a memcmp.
                auto const shared = static_cast<std::ptrdiff_t>(std::ranges::min(x.num_blocks(), y.num_blocks()));
                auto const xf     = std::ranges::begin(x.blocks());
                auto const yf     = std::ranges::begin(y.blocks());
                return std::ranges::equal(xf, xf + shared, yf, yf + shared) and
                       (x.num_blocks() < y.num_blocks()
                                ? not y.any_block_set(x.num_blocks(), y.num_blocks())
                                : not x.any_block_set(y.num_blocks(), x.num_blocks()));
        }
}

// The set ordering across two widths, turning on the lowest position at which the two disagree.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto padded_set_three_way(Bits const& x, Bits const& y) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        auto const n     = std::ranges::max(x.num_blocks(), y.num_blocks());
        auto const index = x.padded_first_difference(y, n);
        if (index == n) {
                return std::strong_ordering::equal;
        }
        auto const diff   = static_cast<block_type>(x.padded_block(index) ^ y.padded_block(index));
        auto const offset = static_cast<std::size_t>(bits::detail::countr_zero(diff));
        if (bits::detail::intersects(x.padded_block(index), shl(block_type{1}, offset))) {
                return y.padded_any_above(index, offset) ? std::strong_ordering::less : std::strong_ordering::greater;
        }
        return x.padded_any_above(index, offset) ? std::strong_ordering::greater : std::strong_ordering::less;
}

// The set reading a block at a time: std::lexicographical_compare_three_way over the ascending positions.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto set_three_way(Bits const& x [[maybe_unused]], Bits const& y [[maybe_unused]]) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        if constexpr (Bits::has_static_size and Bits::extent == 0UZ) {
                return std::strong_ordering::equal;
        } else if constexpr (Bits::has_static_size and Bits::extent == 1UZ) {
                // One position, so the loser is empty and any_above is constantly false.
                return x.test(0UZ) <=> y.test(0UZ);
        } else {
                if constexpr (not Bits::has_static_size) {
                        if (x.size() != y.size()) {
                                return padded_set_three_way(x, y);
                        }
                }
                auto const [index, diff] = x.first_difference(y);
                if (diff == block_type{}) {
                        return std::strong_ordering::equal;
                }
                auto const offset = bits::detail::countr_zero(diff);
                if (bits::detail::intersects(x.block(index), shl(block_type{1}, offset))) {
                        return y.any_above(index, offset) ? std::strong_ordering::less : std::strong_ordering::greater;
                }
                return x.any_above(index, offset) ? std::strong_ordering::greater : std::strong_ordering::less;
        }
}

// The set ordering over descending positions, [associative.reqmts]' lexicographic one: the masks as unsigned numbers.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto numeric_three_way(Bits const& x, Bits const& y) noexcept
        -> std::strong_ordering
{
        // A missing high block reads as zero, so two widths compare aligned at every position.
        auto const n = std::ranges::max(x.num_blocks(), y.num_blocks());
        for (auto index = n - 1UZ; index < n; --index) {
                if (auto const xb = x.padded_block(index), yb = y.padded_block(index); xb != yb) {
                        return xb < yb ? std::strong_ordering::less : std::strong_ordering::greater;
                }
        }
        return std::strong_ordering::equal;
}

// The sequence ordering across two widths: position 0 is the first element, so the lowest disagreement decides alone.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto padded_sequence_three_way(Bits const& x, Bits const& y) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        auto const n     = std::ranges::max(x.num_blocks(), y.num_blocks());
        auto const index = x.padded_first_difference(y, n);
        if (index == n) {
                // The two answers this can give: the caller arrives only with the sizes differing.
                return x.size() < y.size() ? std::strong_ordering::less : std::strong_ordering::greater;
        }
        auto const diff   = static_cast<block_type>(x.padded_block(index) ^ y.padded_block(index));
        auto const offset = static_cast<std::size_t>(bits::detail::countr_zero(diff));
        return bits::detail::intersects(x.padded_block(index), shl(block_type{1}, offset))
                       ? std::strong_ordering::greater
                       : std::strong_ordering::less;
}

// Bools from index 0 a block at a time, for any reading of that order: the holder of the lowest difference is greater.
template<bit_container_type Bits>
[[nodiscard]] constexpr auto sequence_three_way(Bits const& x [[maybe_unused]], Bits const& y [[maybe_unused]]) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        if constexpr (Bits::has_static_size and Bits::extent == 0UZ) {
                return std::strong_ordering::equal;
        } else {
                // A capacity of nought holds only width zero, so there both widths are that.
                if constexpr (not Bits::has_static_size and not Bits::has_zero_capacity) {
                        if (x.size() != y.size()) {
                                return padded_sequence_three_way(x, y);
                        }
                }
                auto const [index, diff] = x.first_difference(y);
                if (diff == block_type{}) {
                        return std::strong_ordering::equal;
                }
                auto const offset = bits::detail::countr_zero(diff);
                return bits::detail::intersects(x.block(index), shl(block_type{1}, offset))
                               ? std::strong_ordering::greater
                               : std::strong_ordering::less;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_COMPARISONS_HPP
