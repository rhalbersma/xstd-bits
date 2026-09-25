//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BORROWED_BLOCK_SPAN_HPP
#define XSTD_BITS_DETAIL_BORROWED_BLOCK_SPAN_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <span>                                    // span
#include <type_traits>                             // is_const_v

namespace xstd::bits::detail {

template<class C>
inline constexpr bool is_block_span = false;

template<class B, std::size_t E>
        requires xstd::unsigned_integer<B> and (not std::is_const_v<B>)
inline constexpr bool is_block_span<std::span<B, E>> = true;

// Blocks someone else owns, written through a span: a handle, so neither regular nor deep-const.
template<class C>
concept borrowed_block_span = is_block_span<C>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BORROWED_BLOCK_SPAN_HPP
