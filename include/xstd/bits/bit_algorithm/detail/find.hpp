//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP

#include <xstd/bits/detail/intrin.hpp> // countl_zero, countr_zero
#include <xstd/bits/detail/shift.hpp>  // partial_block_mask
#include <algorithm>                   // min
#include <cstddef>                     // size_t

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
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        auto const flip       = flip_for<block_type>(value);
        for (auto k = 0UZ; k < len; k += digits) {
                auto const mask  = partial_block_mask<block_type>(std::ranges::min(digits, len - k));
                auto const block = static_cast<block_type>(static_cast<block_type>(c.block_at(n + k) ^ flip) & mask);
                if (block != block_type{}) {
                        return k + countr_zero(block);
                }
        }
        return len;
}

// The last of the len positions from n that holds value, counted from n, or len where none does.
template<class Bits>
[[nodiscard]] constexpr auto find_last_value(Bits const& c, std::size_t n, std::size_t len, bool value) noexcept
        -> std::size_t
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;
        auto const flip       = flip_for<block_type>(value);
        auto const chunks     = (len + digits - 1UZ) / digits;
        for (auto j = chunks - 1UZ; j < chunks; --j) {
                auto const k     = j * digits;
                auto const mask  = partial_block_mask<block_type>(std::ranges::min(digits, len - k));
                auto const block = static_cast<block_type>(static_cast<block_type>(c.block_at(n + k) ^ flip) & mask);
                if (block != block_type{}) {
                        return k + digits - 1UZ - countl_zero(block);
                }
        }
        return len;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_HPP
