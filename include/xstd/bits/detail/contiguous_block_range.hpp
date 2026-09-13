//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <concepts>                                 // regular, same_as
#include <ranges>                                   // contiguous_range, range_const_reference_t, range_reference_t, range_value_t, sized_range

namespace xstd::detail::bits {

// Whether a range IS blocks; block_readable asks if a trait hands a container's blocks over. [design.md#contiguous-block-range]
template<class C>
concept contiguous_block_range =
        std::regular<C> and
        std::ranges::sized_range<C> and
        std::ranges::contiguous_range<C> and
        xstd::unsigned_integer<std::ranges::range_value_t<C>> and
        requires (C& c, C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C>>;
        } and
        // range_const_reference_t<C>, not range_reference_t<C const>: the latter quietly also asks C const to be a range. [design.md#contiguous-block-range]
        requires (C const& c, C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_const_reference_t<C>>;
        }
;

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP
