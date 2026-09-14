//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
#define XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP

#include <version>     // IWYU pragma: keep; __cpp_lib_ranges_as_const
#include <iterator>    // indirectly_readable, iter_reference_t, iter_value_t
#include <ranges>      // iterator_t, range, range_const_reference_t
#include <type_traits> // common_reference_t

namespace xstd::detail::bits {

// P2278R4's two aliases, transcribed from [const.iterators.alias] and [ranges.syn], constraints included: the paper defines them by a formula, so this is a definition and not an approximation of one. The paper's It is spelled I, as every other iterator parameter here is. Unconditional, and in a namespace of its own so that a library shipping the paper does not shadow it but is checked against it. [design.md#the-const-reference]
namespace fallback {

template<std::indirectly_readable I>
using iter_const_reference_t = std::common_reference_t<std::iter_value_t<I> const&&, std::iter_reference_t<I>>;

template<std::ranges::range R>
using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;

}       // namespace fallback

// The standard's where the library has it, the transcription above where it does not. libc++ has implemented P2278R4 on no branch, trunk included: __cpp_lib_ranges_as_const is still a commented-out line in its <version>, and neither as_const_view.h nor const_access.h exists, so a third of the matrix takes the second arm. The arms are the same type rather than two contracts, which is what TheConstReferenceIsP2278s asserts wherever both spellings exist. [design.md#the-const-reference]
#ifdef __cpp_lib_ranges_as_const

template<std::ranges::range R>
using range_const_reference_t = std::ranges::range_const_reference_t<R>;

#else

template<std::ranges::range R>
using range_const_reference_t = fallback::range_const_reference_t<R>;

#endif

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
