//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_DETAIL_BIT_PERMUTE_HPP
#define XSTD_BITS_BIT_DETAIL_BIT_PERMUTE_HPP

#include <xstd/bits/bit/detail/bit_width.hpp>                      // bit_width_v
#include <xstd/bits/bit_valarray/detail/block_compress_expand.hpp> // compress_blocks, compressed, expand_blocks, scatter_assign, stored_masks
#include <xstd/bits/detail/borrowed_bits.hpp>                      // borrowed_bits
#include <xstd/bits/detail/intrinsic.hpp>                          // popcount
#include <xstd/bits/detail/ownership.hpp>                          // owner, reads, set_reading_tag, storage_access
#include <xstd/ints/concepts/unsigned_integer.hpp>                 // unsigned_integer
#include <xstd/ints/limits.hpp>                                    // numeric_limits
#include <cassert>                                                 // assert
#include <cstddef>                                                 // size_t
#include <ranges>                                                  // iota
#include <span>                                                    // dynamic_extent, span
#include <type_traits>                                             // is_const_v, is_volatile_v

// P3104's bit_compress and bit_expand over the blocks of two owners of one type, a block at a time.
namespace xstd::bits::detail {

// One of our owners, its source and its mask of one type, so the result is that type too.
template<class T>
concept permutable = owner<T> and (not std::is_const_v<T>) and (not std::is_volatile_v<T>);

template<class T>
inline constexpr bool permutes_fixed_width = bit_width_v<T> != std::dynamic_extent;

// A sequence's two sizes agree, as a valarray mask's size does; a set reads absent keys past its width.
template<permutable T>
constexpr auto check_widths(T const& x [[maybe_unused]], T const& m [[maybe_unused]]) noexcept
        -> void
{
        if constexpr (not permutes_fixed_width<T> and not reads<T, set_reading_tag>) {
                assert(x.size() == m.size());
        }
}

// An empty set of a run-time width as wide as m, its local at function scope so GCC elides the copy with no branch.
template<permutable T>
[[nodiscard]] constexpr auto empty_set_as_wide_as(T const& m)
        -> T
{
        auto result = T();
        storage_access::bits(result).resize(storage_access::bits(m).size());
        return result;
}

// All false at the width a permutation of the mask's positions lands in: a fixed width, a sequence's size, a set's.
template<permutable T>
[[nodiscard]] constexpr auto zeros_over(T const& m)
        -> T
{
        if constexpr (permutes_fixed_width<T>) {
                return T();
        } else if constexpr (reads<T, set_reading_tag>) {
                return empty_set_as_wide_as(m);
        } else {
                return T(m.size());
        }
}

// The blocks of a source, each past its last block being nought, as a narrower set's absent keys are.
template<class Block>
class padded_blocks
{
        std::span<Block const> m_blocks;

public:
        [[nodiscard]] constexpr explicit padded_blocks(std::span<Block const> blocks) noexcept
                : m_blocks(blocks)
        {}

        [[nodiscard]] constexpr auto operator[](std::size_t k) const noexcept
                -> Block
        {
                return k < m_blocks.size() ? m_blocks[k] : Block{};
        }
};

// The positions of x that m holds, packed in order into the zeros of dst from position zero.
template<class Bits>
constexpr auto compress_into(Bits const& x, Bits const& m, Bits& dst) noexcept
        -> void
{
        using block_type = Bits::block_type;
        auto out         = borrowed_bits<block_type>(dst.blocks());
        compress_blocks(padded_blocks<block_type>(x.blocks()), stored_masks<block_type>(m.blocks()), out);
        dst.recount();
}

// The low bits of x, one per position m holds, laid at those positions in the zeros of dst.
template<class Bits>
constexpr auto expand_into(Bits const& x, Bits const& m, Bits& dst) noexcept
        -> void
{
        using block_type = Bits::block_type;
        auto const in    = x.blocks();
        auto const src   = borrowed_bits<block_type>(std::span(const_cast<block_type*>(in.data()), in.size())); // NOLINT(cppcoreguidelines-pro-type-const-cast): the view is only read.
        auto out         = borrowed_bits<block_type>(dst.blocks());
        expand_blocks(src, stored_masks<block_type>(m.blocks()), out, scatter_assign);
        dst.recount();
}

// The positions of x that m holds, packed into an unsigned integer in a register, with no owner built to hold them.
template<xstd::unsigned_integer U, class Bits>
[[nodiscard]] constexpr auto compress_low(Bits const& x, Bits const& m) noexcept
        -> U
{
        using block_type      = Bits::block_type;
        constexpr auto digits = static_cast<std::size_t>(xstd::numeric_limits<U>::digits);
        auto const source     = padded_blocks<block_type>(x.blocks());
        auto const masks      = m.blocks();
        auto result           = U{};
        auto fill             = 0UZ;
        for (auto const k : std::views::iota(0UZ, masks.size())) {
                // U full means every later mask block is empty, so no shift ever reaches the whole width of U.
                if (fill < digits) {
                        result = static_cast<U>(result | static_cast<U>(static_cast<U>(compressed(source[k], masks[k])) << fill));
                }
                fill += detail::popcount(masks[k]);
        }
        return result;
}

// The blocks of an unsigned integer's low positions, written into the zeros of dst as far as its width reaches.
template<xstd::unsigned_integer U, class Bits>
constexpr auto assign_low(U x, Bits& dst) noexcept
        -> void
{
        using block_type      = Bits::block_type;
        constexpr auto digits = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);
        constexpr auto width  = static_cast<std::size_t>(xstd::numeric_limits<U>::digits);
        auto const blocks     = dst.blocks();
        for (auto k = 0UZ; k < blocks.size() and k * digits < width; ++k) {
                blocks[k] = static_cast<block_type>(x >> (k * digits));
        }
        dst.erase_unused();
        dst.recount();
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_DETAIL_BIT_PERMUTE_HPP
