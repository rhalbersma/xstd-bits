//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_INTERSECTS_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_INTERSECTS_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp> // reads_keys
#include <xstd/bits/detail/block_intersects.hpp>        // block_intersects
#include <xstd/bits/detail/ownership.hpp>               // storage_access
#include <algorithm>                                    // any_of
#include <ranges>                                       // zip
#include <type_traits>                                  // remove_cvref_t, type_identity_t

namespace xstd::ranges {

// Whether std::ranges::set_intersection(s1, s2) would write a key, a block at a time; s2 converts to s1's type.
template<bits::detail::reads_keys S>
[[nodiscard]] constexpr auto bit_intersects(S const& s1, std::type_identity_t<S> const& s2) noexcept
        -> bool
{
        auto const& x = bits::detail::storage_access::bits(s1);
        auto const& y = bits::detail::storage_access::bits(s2);
        using storage = std::remove_cvref_t<decltype(x)>;
        // Only the blocks both storages have can meet, so zip stopping at the shorter is the question.
        if constexpr (storage::has_static_size and storage::static_num_blocks == 0) {
                return false;
        } else if constexpr (storage::has_static_size and storage::static_num_blocks == 1) {
                return bits::detail::block_intersects(x[0], y[0]);
        } else if constexpr (storage::has_static_size and storage::static_num_blocks == 2) {
                return bits::detail::block_intersects(x[0], y[0]) or bits::detail::block_intersects(x[1], y[1]);
        } else {
                return std::ranges::any_of(std::views::zip(x.blocks(), y.blocks()), [](auto&& _) -> bool {
                        auto&& [lhs, rhs] = _;
                        return bits::detail::block_intersects(lhs, rhs);
                });
        }
}

} // namespace xstd::ranges

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_INTERSECTS_HPP
