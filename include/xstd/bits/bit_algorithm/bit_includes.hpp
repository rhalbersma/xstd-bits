//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_BIT_INCLUDES_HPP
#define XSTD_BITS_BIT_ALGORITHM_BIT_INCLUDES_HPP

#include <xstd/bits/bit_algorithm/detail/algorithm.hpp>      // reads_keys
#include <xstd/bits/bit_algorithm/detail/block_includes.hpp> // block_includes
#include <xstd/bits/detail/ownership.hpp>                    // storage_access
#include <algorithm>                                         // all_of
#include <ranges>                                            // zip
#include <type_traits>                                       // remove_cvref_t, type_identity_t

namespace xstd {

// std::ranges::includes(s1, s2), a block at a time; s2 converts to s1's type, as a flag does to its flag set.
// std::ranges::includes(s1, s2), a block at a time; s2 converts to s1's type, as a flag does to its flag set.
template<bits::detail::reads_keys S>
[[nodiscard]] constexpr auto bit_includes(S const& s1, std::type_identity_t<S> const& s2) noexcept
        -> bool
{
        auto const& x = bits::detail::storage_access::bits(s1);
        auto const& y = bits::detail::storage_access::bits(s2);
        using storage = std::remove_cvref_t<decltype(x)>;
        if constexpr (storage::has_static_size and storage::static_num_blocks == 0) {
                return true;
        } else if constexpr (storage::has_static_size and storage::static_num_blocks == 1) {
                return bits::detail::block_includes(x[0], y[0]);
        } else if constexpr (storage::has_static_size and storage::static_num_blocks == 2) {
                return bits::detail::block_includes(x[0], y[0]) and bits::detail::block_includes(x[1], y[1]);
        } else {
                // zip stops at the shorter, which is exactly the blocks both storages have.
                auto const shared = std::ranges::all_of(std::views::zip(x.blocks(), y.blocks()), [](auto&& _) -> bool {
                        auto&& [lhs, rhs] = _;
                        return bits::detail::block_includes(lhs, rhs);
                });
                if constexpr (storage::has_static_size) {
                        // One width, so the shared blocks are all the blocks.
                        return shared;
                } else {
                        // Above the shared blocks only s2 can hold a key, which s1 then lacks.
                        return shared and not y.any_block_set(x.num_blocks(), y.num_blocks());
                }
        }
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ALGORITHM_BIT_INCLUDES_HPP
