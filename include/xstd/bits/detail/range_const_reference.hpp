//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
#define XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP

#include <iterator>    // indirectly_readable, iter_reference_t, iter_value_t
#include <ranges>      // iterator_t, range, range_const_reference_t
#include <type_traits> // common_reference_t
#include <version>     // IWYU pragma: keep; __cpp_lib_ranges_as_const

namespace xstd::detail::bits {

// P2278R4's two aliases, transcribed from [const.iterators.alias] and [ranges.syn], constraints included.
namespace fallback {

template<std::indirectly_readable I>
using iter_const_reference_t = std::common_reference_t<std::iter_value_t<I> const&&, std::iter_reference_t<I>>;

template<std::ranges::range R>
using range_const_reference_t = iter_const_reference_t<std::ranges::iterator_t<R>>;

} // namespace fallback

// The standard's alias where the library ships P2278R4, the transcription where it does not; both name one type.
#ifdef __cpp_lib_ranges_as_const

template<std::ranges::range R>
using range_const_reference_t = std::ranges::range_const_reference_t<R>;

#else

template<std::ranges::range R>
using range_const_reference_t = fallback::range_const_reference_t<R>;

#endif

} // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
