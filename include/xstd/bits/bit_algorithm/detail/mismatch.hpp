//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP

#include <xstd/bits/detail/intrinsic.hpp> // countr_zero
#include <algorithm>                      // min
#include <cstddef>                        // size_t

namespace xstd::bits::detail {

// The first of the shorter's n positions where the two differ, or n: a difference in its padding lies past n.
template<class Bits>
[[nodiscard]] constexpr auto first_mismatch(Bits const& shorter, Bits const& longer, std::size_t n) noexcept
        -> std::size_t
{
        if (n == 0UZ) {
                return 0UZ;
        }
        auto const [index, diff] = shorter.first_difference(longer);
        if (diff == typename Bits::block_type{}) {
                return n;
        }
        return std::min((index * Bits::bits_per_block) + static_cast<std::size_t>(countr_zero(diff)), n);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP
