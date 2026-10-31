//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_IF_HPP
#define XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_IF_HPP

#include <xstd/bits/bit_algorithm/detail/walk.hpp> // decay_copy
#include <xstd/bits/detail/intrinsic.hpp>          // countl_zero, countr_zero
#include <xstd/bits/detail/key_order.hpp>          // descends
#include <xstd/bits/detail/ownership.hpp>          // storage_access
#include <xstd/bits/detail/walk_block.hpp>         // drop_highest, drop_lowest
#include <functional>                              // invoke
#include <optional>                                // nullopt, optional
#include <ranges>                                  // iota
#include <type_traits>                             // remove_cvref_t

namespace xstd::bits::detail {

// A set's first key, in the order it iterates, that pred accepts: the walk's block loop, left at the first hit.
template<class R, class Pred>
[[nodiscard]] constexpr auto find_key_if(R const& r, Pred& pred)
        -> std::optional<typename std::remove_cvref_t<R>::key_type>
{
        using key_type        = std::remove_cvref_t<R>::key_type;
        using mapping_type    = std::remove_cvref_t<R>::key_mapping_type;
        auto const& c         = storage_access::bits(r);
        using bits_type       = std::remove_cvref_t<decltype(c)>;
        using block_type      = bits_type::block_type;
        constexpr auto digits = bits_type::bits_per_block;
        if constexpr (descends<R>) {
                auto const blocks = c.num_blocks();
                for (auto index = blocks - 1UZ; index < blocks; --index) {
                        auto block = c[index];
                        while (block != block_type{}) {
                                auto const offset = digits - 1UZ - countl_zero(block);
                                auto const key    = decay_copy<key_type>(mapping_type::from_index((digits * index) + offset));
                                if (std::invoke(pred, key)) {
                                        return key;
                                }
                                block = drop_highest(block);
                        }
                }
        } else {
                for (auto const index : std::views::iota(0UZ, c.num_blocks())) {
                        auto block = c[index];
                        while (block != block_type{}) {
                                auto const key = decay_copy<key_type>(mapping_type::from_index((digits * index) + countr_zero(block)));
                                if (std::invoke(pred, key)) {
                                        return key;
                                }
                                block = drop_lowest(block);
                        }
                }
        }
        return std::nullopt;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_ALGORITHM_DETAIL_FIND_IF_HPP
