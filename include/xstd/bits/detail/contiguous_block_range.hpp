//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP
#define XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP

#include <xstd/bits/detail/range_const_reference.hpp> // range_const_reference_t
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <concepts>                                   // regular, same_as
#include <cstddef>                                    // size_t
#include <ranges>                                     // contiguous_range, range_reference_t, range_value_t, sized_range
#include <span>                                       // span
#include <type_traits>                                // is_const_v

namespace xstd::bits::detail {

// Whether a range IS blocks; block_readable asks if a trait hands a container's blocks over.
template<class C>
concept contiguous_block_range =
        std::regular<C> and
        std::ranges::sized_range<C> and
        std::ranges::contiguous_range<C> and
        xstd::unsigned_integer<std::ranges::range_value_t<C>> and
        requires (C& c, C::size_type n) {
                { c[n] } -> std::same_as<std::ranges::range_reference_t<C>>;
        } and
        // P2278R4's alias, transcribed: a storage whose const subscript yields a writable reference is refused here.
        requires (C const& c, C::size_type n) {
                { c[n] } -> std::same_as<range_const_reference_t<C>>;
        };

template<class C>
inline constexpr bool is_block_span = false;

template<class B, std::size_t E>
        requires xstd::unsigned_integer<B> and (not std::is_const_v<B>)
inline constexpr bool is_block_span<std::span<B, E>> = true;

// Blocks someone else owns, written through a span: a handle, so neither regular nor deep-const.
template<class C>
concept borrowed_block_span = is_block_span<C>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_CONTIGUOUS_BLOCK_RANGE_HPP
