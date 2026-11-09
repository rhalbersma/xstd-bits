//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP

#include <xstd/bits/detail/block_chunks.hpp> // find_chunk, find_chunk_down
#include <xstd/bits/detail/intrinsic.hpp>    // countl_zero, countr_zero
#include <xstd/bits/detail/shift.hpp>        // partial_block_mask
#include <cstddef>                           // size_t

namespace xstd::bits::detail {

// All ones where value is false, all zeros where true: an xor turns a search for false into one for true.
template<class Block>
[[nodiscard]] constexpr auto flip_for(bool value) noexcept
        -> Block
{
        return value ? Block{} : static_cast<Block>(~Block{});
}

// The first of the len positions from n that holds value, counted from n, or len where none does.
template<class Bits>
[[nodiscard]] constexpr auto find_value(Bits const& c, std::size_t n, std::size_t len, bool value) noexcept
        -> std::size_t
{
        using block_type = Bits::block_type;
        auto const flip  = flip_for<block_type>(value);
        auto found       = len;
        find_chunk<Bits::bits_per_block>(n, len, [&](std::size_t k, std::size_t count) -> bool {
                auto const block = static_cast<block_type>(static_cast<block_type>(c.block_at(n + k) ^ flip) & partial_block_mask<block_type>(count));
                if (block == block_type{}) {
                        return false;
                }
                found = k + countr_zero(block);
                return true;
        });
        return found;
}

// The last of the len positions from n that holds value, counted from n, or len where none does.
template<class Bits>
[[nodiscard]] constexpr auto find_last_value(Bits const& c, std::size_t n, std::size_t len, bool value) noexcept
        -> std::size_t
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        auto const flip       = flip_for<block_type>(value);
        auto found            = len;
        find_chunk_down<digits>(n, len, [&](std::size_t k, std::size_t count) -> bool {
                auto const block = static_cast<block_type>(static_cast<block_type>(c.block_at(n + k) ^ flip) & partial_block_mask<block_type>(count));
                if (block == block_type{}) {
                        return false;
                }
                found = k + digits - 1UZ - countl_zero(block);
                return true;
        });
        return found;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP
