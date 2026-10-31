//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_GATHER_INDICES_HPP
#define XSTD_BITS_DETAIL_GATHER_INDICES_HPP

#include <xstd/bits/detail/bit_at.hpp>             // bit_at
#include <xstd/bits/detail/block_width.hpp>        // block_width
#include <xstd/bits/detail/borrowed_bits.hpp>      // borrowed_bits
#include <xstd/bits/detail/shift.hpp>              // shl
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota

namespace xstd::bits::detail {

// The bits at index(0), ..., index(count - 1) of src, in order, into dst from position zero, each block stored once.
template<class Src, xstd::unsigned_integer Block, class Index>
constexpr auto gather_indices(Src const& src, std::size_t count, Index index, borrowed_bits<Block>& dst) noexcept
        -> void
{
        auto acc = Block{};
        for (auto const j : std::views::iota(0UZ, count)) {
                acc = static_cast<Block>(acc | shl(bit_at(src, index(j)), j % block_width<Block>));
                if (j % block_width<Block> == block_width<Block> - 1UZ) {
                        dst[j / block_width<Block>] = acc;
                        acc                         = Block{};
                }
        }
        if (count % block_width<Block> != 0UZ) {
                dst[count / block_width<Block>] = acc;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_GATHER_INDICES_HPP
