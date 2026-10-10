//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_COMPARISONS_HPP
#define XSTD_BITS_DETAIL_COMPARISONS_HPP

#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container_type
#include <xstd/bits/detail/intrin.hpp>              // countr_zero
#include <xstd/bits/detail/ownership.hpp>           // sequence_reading_tag, set_reading_tag
#include <xstd/bits/detail/pred.hpp>                // intersects
#include <xstd/bits/detail/shift.hpp>               // shl
#include <algorithm>                                // equal, max, min
#include <compare>                                  // strong_ordering
#include <concepts>                                 // same_as
#include <cstddef>                                  // ptrdiff_t, size_t
#include <ranges>                                   // begin

// Each reading's equality and ordering over a bit_block_container, spelled in its primitives and named for the reading.
namespace xstd::bits::detail {

// The set reading's equality, where width is capacity: neither value being the subject, it takes both.
template<bit_block_container_type Bits>
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

// The set ordering over descending positions, [associative.reqmts]' lexicographic one: the masks as unsigned numbers.
template<bit_block_container_type Bits>
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

// The order at the lowest position where two readings first differ: a sequence reads it as the most significant.
template<class Reading, class Bits, class AnyAbove>
[[nodiscard]] constexpr auto at_first_difference(bool x_holds, Bits const& x [[maybe_unused]], Bits const& y [[maybe_unused]], AnyAbove any_above [[maybe_unused]]) noexcept
        -> std::strong_ordering
{
        if constexpr (std::same_as<Reading, sequence_reading_tag>) {
                return x_holds ? std::strong_ordering::greater : std::strong_ordering::less;
        } else if (x_holds) {
                // A set's keys ascend, so holding the lower key wins unless the other set still holds one above it.
                return any_above(y) ? std::strong_ordering::less : std::strong_ordering::greater;
        } else {
                return any_above(x) ? std::strong_ordering::greater : std::strong_ordering::less;
        }
}

// Either ordering across two widths, the shorter padded with zero blocks.
template<class Reading, bit_block_container_type Bits>
[[nodiscard]] constexpr auto padded_three_way(Bits const& x, Bits const& y) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        auto const n     = std::ranges::max(x.num_blocks(), y.num_blocks());
        auto const index = x.padded_first_difference(y, n);
        if (index == n) {
                if constexpr (std::same_as<Reading, sequence_reading_tag>) {
                        // Equal over the padding, so the shorter sequence is a prefix of the longer.
                        return x.size() < y.size() ? std::strong_ordering::less : std::strong_ordering::greater;
                } else {
                        // The padding holds no key, so the two sets hold the same keys.
                        return std::strong_ordering::equal;
                }
        }
        auto const diff   = static_cast<block_type>(x.padded_block(index) ^ y.padded_block(index));
        auto const offset = static_cast<std::size_t>(bits::detail::countr_zero(diff));
        return at_first_difference<Reading>(bits::detail::intersects(x.padded_block(index), shl(block_type{1}, offset)), x, y, [&](Bits const& b) -> bool { return b.padded_any_above(index, offset); });
}

// lexicographical_compare_three_way over a set's ascending keys or a sequence's bools, a block at a time.
template<class Reading, bit_block_container_type Bits>
        requires std::same_as<Reading, set_reading_tag> or std::same_as<Reading, sequence_reading_tag>
[[nodiscard]] constexpr auto three_way(Bits const& x [[maybe_unused]], Bits const& y [[maybe_unused]]) noexcept
        -> std::strong_ordering
{
        using block_type = Bits::block_type;
        if constexpr (Bits::has_static_size and Bits::extent == 0UZ) {
                return std::strong_ordering::equal;
        } else if constexpr (Bits::has_static_size and Bits::extent == 1UZ) {
                // One position, where both readings order false before true.
                return x.test(0UZ) <=> y.test(0UZ);
        } else {
                // A capacity of nought holds only width zero, so there both widths are that.
                if constexpr (not Bits::has_static_size and not Bits::has_zero_capacity) {
                        if (x.size() != y.size()) {
                                return padded_three_way<Reading>(x, y);
                        }
                }
                auto const [index, diff] = x.first_difference(y);
                if (diff == block_type{}) {
                        return std::strong_ordering::equal;
                }
                auto const offset = bits::detail::countr_zero(diff);
                return at_first_difference<Reading>(bits::detail::intersects(x[index], shl(block_type{1}, offset)), x, y, [&](Bits const& b) -> bool { return b.any_above(index, offset); });
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_COMPARISONS_HPP
