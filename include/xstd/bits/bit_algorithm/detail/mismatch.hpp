//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP

#include <xstd/bits/bit_algorithm/detail/blit.hpp> // read_block
#include <xstd/bits/detail/block_chunks.hpp>       // find_chunk
#include <xstd/bits/detail/intrinsic.hpp>          // countr_zero
#include <xstd/bits/detail/shift.hpp>              // partial_block_mask
#include <cstddef>                                 // size_t

namespace xstd::bits::detail {

// The first of the len positions where x from position i and y from position j differ, or len: a block of x at a time.
template<class X, class Y>
[[nodiscard]] constexpr auto mismatch_bools(X const& x, std::size_t i, Y const& y, std::size_t j, std::size_t len) noexcept
        -> std::size_t
{
        using block_type = X::block_type;
        auto found       = len;
        find_chunk<X::bits_per_block>(i, len, [&](std::size_t k, std::size_t count) -> bool {
                auto const diff = static_cast<block_type>(static_cast<block_type>(read_block<block_type>(x, i + k, count) ^ read_block<block_type>(y, j + k, count)) & partial_block_mask<block_type>(count));
                if (diff == block_type{}) {
                        return false;
                }
                found = k + countr_zero(diff);
                return true;
        });
        return found;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_MISMATCH_HPP
