//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <concepts>                                 // regular, same_as
#include <ranges>                                   // contiguous_range, range_reference_t, range_value_t, sized_range

namespace xstd::detail::bits {

// Whether a range IS blocks; block_readable asks if a trait hands a container's blocks over. [design.md#contiguous-block-container]
template<class C>
concept contiguous_block_container =
        std::regular<C> and
        std::ranges::sized_range<C> and
        std::ranges::contiguous_range<C> and
        xstd::unsigned_integer<std::ranges::range_value_t<C>> and
        requires (C& c, C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C>>;
        } and
        requires (C const& c, C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C const>>;
        }
;

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP
