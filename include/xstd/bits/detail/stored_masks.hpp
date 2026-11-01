//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_STORED_MASKS_HPP
#define XSTD_BITS_DETAIL_STORED_MASKS_HPP

#include <xstd/bits/detail/intrinsic.hpp>          // popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <ranges>                                  // iota
#include <span>                                    // span

namespace xstd::bits::detail {

// The masks a mask array stores, one per block.
template<xstd::unsigned_integer Block>
class stored_masks
{
        std::span<Block const> m_masks;

public:
        [[nodiscard]] constexpr explicit stored_masks(std::span<Block const> masks) noexcept
                : m_masks(masks)
        {}

        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                for (auto const k : std::views::iota(0UZ, m_masks.size())) {
                        f(k, m_masks[k], detail::popcount(m_masks[k]));
                }
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_STORED_MASKS_HPP
