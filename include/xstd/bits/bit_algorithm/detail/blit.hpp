//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_BLIT_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_BLIT_HPP

#include <xstd/bits/detail/shift.hpp> // partial_block_mask, shl, shr
#include <algorithm>                  // min
#include <concepts>                   // convertible_to, invocable, same_as
#include <cstddef>                    // size_t
#include <type_traits>                // invoke_result_t, remove_cvref_t

namespace xstd::bits::detail {

// The count bits of c from position n at the bottom of a Block: c's own block, or c's blocks assembled or cut.
template<class Block, class Bits>
[[nodiscard]] constexpr auto read_block(Bits const& c, std::size_t n, std::size_t count) noexcept
        -> Block
{
        if constexpr (std::same_as<Block, typename Bits::block_type>) {
                return c.block_at(n);
        } else {
                auto block = Block{};
                for (auto j = 0UZ; j < count; j += Bits::bits_per_block) {
                        block = static_cast<Block>(block | shl(static_cast<Block>(c.block_at(n + j)), j));
                }
                return block;
        }
}

// The len positions of dst from d written a block at a time, lowest first, block(k, count) giving those from k.
template<class Dst, class F>
constexpr auto write_up(Dst&& dst, std::size_t d, std::size_t len, F block) noexcept
        -> void
{
        using block_type      = std::remove_cvref_t<Dst>::block_type;
        constexpr auto digits = std::remove_cvref_t<Dst>::bits_per_block;
        for (auto k = 0UZ; k < len; k += digits) {
                auto const count = std::ranges::min(digits, len - k);
                dst.block_at(d + k, block(k, count), partial_block_mask<block_type>(count));
        }
}

// The mirror, highest first, so a destination above its source never overwrites what is still to be read.
template<class Dst, class F>
constexpr auto write_down(Dst&& dst, std::size_t d, std::size_t len, F block) noexcept
        -> void
{
        using block_type      = std::remove_cvref_t<Dst>::block_type;
        constexpr auto digits = std::remove_cvref_t<Dst>::bits_per_block;
        auto const chunks     = (len + digits - 1UZ) / digits;
        for (auto j = chunks - 1UZ; j < chunks; --j) {
                auto const count = std::ranges::min(digits, len - (j * digits));
                dst.block_at(d + (j * digits), block(j * digits, count), partial_block_mask<block_type>(count));
        }
}

// The len bools of src from position s, as copy writes them to dst from position d: lowest first, or Backward highest.
template<bool Backward = false, class Dst, class Src>
constexpr auto copy_bools(Dst&& dst, std::size_t d, Src const& src, std::size_t s, std::size_t len) noexcept
        -> void
{
        using block_type = std::remove_cvref_t<Dst>::block_type;
        auto const read  = [&](std::size_t k, std::size_t count) -> block_type { return read_block<block_type>(src, s + k, count); };
        if constexpr (Backward) {
                write_down(dst, d, len, read);
        } else {
                write_up(dst, d, len, read);
        }
}

// The len bools of src from position s, last first, written to dst from position d.
template<class Dst, class Src>
constexpr auto reverse_copy_bools(Dst&& dst, std::size_t d, Src const& src, std::size_t s, std::size_t len) noexcept
        -> void
{
        using bits_type       = std::remove_cvref_t<Dst>;
        using block_type      = bits_type::block_type;
        constexpr auto digits = bits_type::bits_per_block;
        write_up(dst, d, len, [&](std::size_t k, std::size_t count) -> block_type {
                // The count read from below the reversed position k land at the top, and come down to the bottom.
                return shr(bits_type::reversed(read_block<block_type>(src, s + len - k - count, count)), digits - count);
        });
}

// The len bools of x from position i and of y from position j traded, a block of the narrower at a time.
template<class X, class Y>
constexpr auto swap_bools(X&& x, std::size_t i, Y&& y, std::size_t j, std::size_t len) noexcept
        -> void
{
        using x_block       = std::remove_cvref_t<X>::block_type;
        using y_block       = std::remove_cvref_t<Y>::block_type;
        constexpr auto step = std::ranges::min(std::remove_cvref_t<X>::bits_per_block, std::remove_cvref_t<Y>::bits_per_block);
        for (auto k = 0UZ; k < len; k += step) {
                auto const count  = std::ranges::min(step, len - k);
                auto const from_x = read_block<y_block>(x, i + k, count);
                x.block_at(i + k, read_block<x_block>(y, j + k, count), partial_block_mask<x_block>(count));
                y.block_at(j + k, from_x, partial_block_mask<y_block>(count));
        }
}

// A function of one bool as a block expression: asked of false and of true, it is a constant, the identity or not.
template<class Block, class F>
[[nodiscard]] constexpr auto unary_table(F& f)
        -> auto
{
        auto const at = [&](bool x) -> Block { return static_cast<bool>(f(x)) ? static_cast<Block>(~Block{}) : Block{}; };
        return [if_false = at(false), if_true = at(true)](Block x) noexcept -> Block {
                return static_cast<Block>(static_cast<Block>(x & if_true) | static_cast<Block>(static_cast<Block>(~x) & if_false));
        };
}

// A function of two bools as a block expression: its truth table, asked once at each of the four pairs.
template<class Block, class F>
[[nodiscard]] constexpr auto binary_table(F& f)
        -> auto
{
        auto const at = [&](bool x, bool y) -> Block { return static_cast<bool>(f(x, y)) ? static_cast<Block>(~Block{}) : Block{}; };
        return [t00 = at(false, false), t01 = at(false, true), t10 = at(true, false), t11 = at(true, true)](Block x, Block y) noexcept -> Block {
                auto const nx      = static_cast<Block>(~x);
                auto const ny      = static_cast<Block>(~y);
                auto const both    = static_cast<Block>(static_cast<Block>(x & y) & t11);
                auto const x_only  = static_cast<Block>(static_cast<Block>(x & ny) & t10);
                auto const y_only  = static_cast<Block>(static_cast<Block>(nx & y) & t01);
                auto const neither = static_cast<Block>(static_cast<Block>(nx & ny) & t00);
                return static_cast<Block>(static_cast<Block>(both | x_only) | static_cast<Block>(y_only | neither));
        };
}

// A function std::ranges::transform could write into a sequence of bools: called with bools, answering a bool.
template<class F, class... Args>
concept bool_function = std::invocable<F&, Args...> and std::convertible_to<std::invoke_result_t<F&, Args...>, bool>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_BLIT_HPP
