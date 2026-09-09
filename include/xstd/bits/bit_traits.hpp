//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TRAITS_HPP
#define XSTD_BITS_BIT_TRAITS_HPP

#include <xstd/bits/detail/intrin.hpp>             // countl_zero, countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cassert>                                 // assert
#include <concepts>                                // convertible_to
#include <cstddef>                                 // size_t
#include <limits>                                  // digits
#include <span>                                    // dynamic_extent
#include <type_traits>                             // remove_cvref_t

// The one point of adaptation: everything above asks bit_traits<Bits> and never the type itself. [design.md#the-trait]
namespace xstd {

// Declared, never defined: an unadapted type is an error, not a silent fallback. [design.md#opt-in]
template<class Bits>
struct bit_traits;

// Whether the trait offers block access; libc++'s std::bitset and boost do not. [design.md#detection-by-absence]
template<class Traits, class Bits>
concept block_readable =
        requires (Bits const& c, std::size_t i)
        {
                { Traits::num_blocks(c) } -> std::convertible_to<std::size_t>;
                { Traits::block(c, i)   } -> xstd::unsigned_integer;
        }
;

} // namespace xstd

// Free functions so block_readable stays answerable, nested so no unqualified call reaches ADL. [design.md#why-nested]
namespace xstd::detail::bits {

// Derived, not declared; here so a degenerate width gets different code. [design.md#per-instantiation-slots]
template<class Traits, class Block>
[[nodiscard]] consteval auto static_block_count() noexcept
        -> std::size_t
{
        if constexpr (Traits::extent == std::dynamic_extent) {
                return std::dynamic_extent;
        } else {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Block>::digits);
                return Traits::extent == 0UZ ? 1UZ : ((Traits::extent + digits - 1UZ) / digits);
        }
}

// One function per tier: a shared body exceeds the complexity threshold. [design.md#one-function-per-tier]

// The last set position at or below i, block at a time.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_prev_by_block(Bits const& c, std::size_t i) noexcept
        -> std::size_t
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
        constexpr auto left_bit = digits - 1UZ;

        auto const start = i / digits;
        auto const offset = i % digits;

        // Shifted up rather than masked so countl_zero measures back down; at left_bit that shift is zero.
        if (auto const block = static_cast<block_type>(Traits::block(c, start) << (left_bit - offset)); block != block_type{}) {
                return i - detail::bits::countl_zero(block);
        }
        // Guarded, cursor inside: one block makes both unreachable. [design.md#per-instantiation-slots]
        if constexpr (static_block_count<Traits, block_type>() != 1UZ) {
                auto index = start;
                while (index-- != 0UZ) {
                        if (auto const block = Traits::block(c, index); block != block_type{}) {
                                return (digits * index) + left_bit - detail::bits::countl_zero(block);
                        }
                }
        }
        return Traits::size(c);
}

// The same, one position at a time, for a type that keeps its blocks to itself.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_prev_by_element(Bits const& c, std::size_t i) noexcept
        -> std::size_t
{
        // One test, not a loop: a loop's exit arm is untakeable here. [design.md#per-instantiation-slots]
        if constexpr (Traits::extent == 1UZ) {
                return Traits::at(c, i) ? i : Traits::size(c);
        } else {
                for (;;) {
                        if (Traits::at(c, i)) {
                                return i;
                        }
                        if (i == 0UZ) {
                                return Traits::size(c);
                        }
                        --i;
                }
        }
}

// The first set position at or above n, block at a time.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_next_by_block(Bits const& c, std::size_t n) noexcept
        -> std::size_t
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        auto const start = n / digits;
        auto const offset = n % digits;

        // No offset != 0 guard: >> 0 is the identity, and #88 measured it as pure cost. [design.md#offset-guards]
        if (auto const block = static_cast<block_type>(Traits::block(c, start) >> offset); block != block_type{}) {
                return n + detail::bits::countr_zero(block);
        }
        // The mirror of scan_prev_by_block's guard, cursor included, for the same two reasons.
        if constexpr (static_block_count<Traits, block_type>() != 1UZ) {
                for (auto index = start + 1UZ, blocks = Traits::num_blocks(c); index < blocks; ++index) {
                        if (auto const block = Traits::block(c, index); block != block_type{}) {
                                return (digits * index) + detail::bits::countr_zero(block);
                        }
                }
        }
        return Traits::size(c);
}

// The same, one position at a time.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_next_by_element(Bits const& c, std::size_t n) noexcept
        -> std::size_t
{
        auto const size = Traits::size(c);
        for (auto i = n; i < size; ++i) {
                if (Traits::at(c, i)) {
                        return i;
                }
        }
        return size;
}

// The primitive both forward scans derive from, by + 1 and never - 1. [design.md#inclusive-is-the-primitive]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_inclusive_next(Bits const& c, std::size_t n) noexcept
        -> std::size_t
{
        auto const size = Traits::size(c);
        if constexpr (Traits::extent == 0UZ) {
                return size;
        } else {
                if (n >= size) {
                        return size;
                }
                if constexpr (block_readable<Traits, Bits>) {
                        return scan_next_by_block<Traits>(c, n);
                } else {
                        return scan_next_by_element<Traits>(c, n);
                }
        }
}

// The first set position, or size(). Total, like every scan here. [design.md#total-versus-precondition]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_first(Bits const& c) noexcept
        -> std::size_t
{
        return scan_inclusive_next<Traits>(c, 0UZ);
}

// One past the last position, which for every reading is the width: not a scan, but asked for alongside them.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_last(Bits const& c) noexcept
        -> std::size_t
{
        return Traits::size(c);
}

// Strictly above n, and total where block_sequence's twin is not. [design.md#total-versus-precondition]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_next(Bits const& c, std::size_t n) noexcept
        -> std::size_t
{
        auto const size = Traits::size(c);
        return n >= size ? size : scan_inclusive_next<Traits>(c, n + 1UZ);
}

// The last set position below n, total: a fallback has no reverse iteration to guard it. [design.md#total-versus-precondition]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_prev(Bits const& c, std::size_t n) noexcept
        -> std::size_t
{
        auto const size = Traits::size(c);
        // A zero width holds nothing below anything, said here to leave no unreachable arm. [design.md#per-instantiation-slots]
        if constexpr (Traits::extent == 0UZ) {
                return size;
        } else {
                auto const i = n < size ? n : size;
                if (i == 0UZ) {
                        return size;
                }
                // The tier is a complexity fix, not an optimization: element-wise reverse traversal is O(N^2).
                if constexpr (block_readable<Traits, Bits>) {
                        return scan_prev_by_block<Traits>(c, i - 1UZ);
                } else {
                        return scan_prev_by_element<Traits>(c, i - 1UZ);
                }
        }
}

// How many positions are set; the block tier is the whole point, popcount per word rather than a test per bit.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto scan_count(Bits const& c) noexcept
        -> std::size_t
{
        if constexpr (Traits::extent == 0UZ) {
                return 0UZ;
        } else if constexpr (block_readable<Traits, Bits>) {
                auto n = 0UZ;
                for (auto i = 0UZ, blocks = Traits::num_blocks(c); i < blocks; ++i) {
                        n += detail::bits::popcount(Traits::block(c, i));
                }
                return n;
        } else {
                auto n = 0UZ;
                for (auto i = 0UZ, size = Traits::size(c); i < size; ++i) {
                        n += Traits::at(c, i) ? 1UZ : 0UZ;
                }
                return n;
        }
}

// The block-wide word at any position, aligned or not: the bits [pos, pos + digits) of c, read through the trait, so a source's alignment is the reader's problem and never the writer's. [design.md#the-blit]
// pos must lie within the blocks; what the word reaches beyond the width is the clear tail, and beyond the last block nothing at all.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto word_at(Bits const& c, std::size_t pos) noexcept
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);
        auto const index = pos / digits;
        auto const offset = pos % digits;
        assert(index < Traits::num_blocks(c));
        auto const low = static_cast<block_type>(Traits::block(c, index) >> offset);
        // No shift by digits, which is undefined: an aligned read is the block itself, and the last block has nothing above it.
        if (offset == 0UZ or index + 1UZ == Traits::num_blocks(c)) {
                return low;
        }
        return static_cast<block_type>(low | static_cast<block_type>(Traits::block(c, index + 1UZ) << (digits - offset)));
}

// A zero width answers zero to every question, and says so here, before an entry or a walk is instantiated for it. [design.md#degenerate-widths]
template<class Traits>
constexpr bool zero_width = Traits::extent == 0UZ;

// The trait's entry where the specialization declares one, the generic scan above where it does not. [design.md#detection-by-absence]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto find_first(Bits const& c [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (zero_width<Traits>) {
                return 0UZ;
        } else if constexpr (requires { { Traits::find_first(c) } -> std::convertible_to<std::size_t>; }) {
                return Traits::find_first(c);
        } else {
                return scan_first<Traits>(c);
        }
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto count(Bits const& c [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (zero_width<Traits>) {
                return 0UZ;
        } else if constexpr (requires { { Traits::count(c) } -> std::convertible_to<std::size_t>; }) {
                return Traits::count(c);
        } else {
                return scan_count<Traits>(c);
        }
}

// The three the sequence reading asks in its own vocabulary, each behind an entry where the storage has one and
// synthesized where it does not. Neither synthesis walks a bit at a time that it could avoid: any is a scan that
// stops at the first set position, and all counts, which is the block tier through count's own door.
// [design.md#the-sequence-aggregates]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto any(Bits const& c [[maybe_unused]]) noexcept
        -> bool
{
        if constexpr (zero_width<Traits>) {
                return false;
        } else if constexpr (requires { { Traits::any(c) } -> std::convertible_to<bool>; }) {
                return Traits::any(c);
        } else {
                return find_first<Traits>(c) != Traits::size(c);
        }
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto none(Bits const& c [[maybe_unused]]) noexcept
        -> bool
{
        if constexpr (zero_width<Traits>) {
                return true;
        } else if constexpr (requires { { Traits::none(c) } -> std::convertible_to<bool>; }) {
                return Traits::none(c);
        } else {
                return not any<Traits>(c);
        }
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto all(Bits const& c [[maybe_unused]]) noexcept
        -> bool
{
        if constexpr (zero_width<Traits>) {
                return true;
        } else if constexpr (requires { { Traits::all(c) } -> std::convertible_to<bool>; }) {
                return Traits::all(c);
        } else {
                return count<Traits>(c) == Traits::size(c);
        }
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto find_next(Bits const& c [[maybe_unused]], std::size_t n [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (zero_width<Traits>) {
                return 0UZ;
        } else if constexpr (requires { { Traits::find_next(c, n) } -> std::convertible_to<std::size_t>; }) {
                return Traits::find_next(c, n);
        } else {
                return scan_next<Traits>(c, n);
        }
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto find_prev(Bits const& c [[maybe_unused]], std::size_t n [[maybe_unused]]) noexcept
        -> std::size_t
{
        if constexpr (zero_width<Traits>) {
                return 0UZ;
        } else if constexpr (requires { { Traits::find_prev(c, n) } -> std::convertible_to<std::size_t>; }) {
                return Traits::find_prev(c, n);
        } else {
                return scan_prev<Traits>(c, n);
        }
}

} // namespace xstd::detail::bits

namespace xstd {

// The three required entries, gated as a concept so an unadapted type reads "constraint not satisfied". [design.md#opt-in]
// Two parameters like block_readable, so a type-constraint can name the trait: bit_storage<Bits> Traits. [design.md#the-trait-is-a-parameter]
template<class Traits, class Bits>
concept bit_storage =
        requires (Bits const& c, std::size_t n)
        {
                // Required rather than optional, so static_bit_extent can read it without proving it exists.
                { Traits::extent   } -> std::convertible_to<std::size_t>;
                { Traits::size(c)  } -> std::convertible_to<std::size_t>;
                { Traits::at(c, n) } -> std::convertible_to<bool>;
        }
;

// A static width reaching the readings as a constant expression; replaces the bit_extent variable template.
template<class Traits, class Bits>
concept static_bit_extent = bit_storage<Traits, Bits> and Traits::extent != std::dynamic_extent;

} // namespace xstd

#endif // XSTD_BITS_BIT_TRAITS_HPP
