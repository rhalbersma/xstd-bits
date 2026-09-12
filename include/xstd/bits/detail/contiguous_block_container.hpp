//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP

#include <xstd/ints/concepts/bitwise_operators.hpp> // bitwise_operators
#include <concepts>                                 // regular, same_as
#include <ranges>                                   // contiguous_range, range_reference_t, range_value_t, sized_range

namespace xstd::detail::bits {

// Whether a range IS blocks; block_readable asks if a trait hands a container's blocks over. [design.md#contiguous-block-container]
// The element clause is bitwise_operators rather than unsigned_integer: what a block is asked for AS a block is
// the operator set std::bitset generalized from the built-in integers, and that is the concept naming it.
// [design.md#contiguous-block-container]
// Subscript is spelled out because contiguous_range promises data() and the ITERATOR's operator[], never the range's,
// and a contiguous container generalizes a C array, whose defining operation is a[n]. [design.md#contiguous-block-container]
// Semantic requirement, as random_access_iterator states for its own i[n]: c[n] is *(std::ranges::begin(c) + n).
// The index is the container's own size_type, as [sequence.reqmts] spells a[n], and not a bare std::size_t: a
// contiguous BLOCK CONTAINER is a container, and every storage this is instantiated over is one of the three
// std containers, each of which names it. [design.md#contiguous-block-container]
// Two requires-expressions rather than one over three parameters: the mutable and the const subscript are separate
// requirements, and each parameter list then names only what its own expression uses. [design.md#contiguous-block-container]
template<class C>
concept contiguous_block_container =
        std::regular<C> and
        std::ranges::sized_range<C> and
        std::ranges::contiguous_range<C> and
        xstd::bitwise_operators<std::ranges::range_value_t<C>> and
        requires (C& c, typename C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C>>;
        } and
        requires (C const& c, typename C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C const>>;
        }
;

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_CONTAINER_HPP
