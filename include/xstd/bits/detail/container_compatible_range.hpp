//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_CONTAINER_COMPATIBLE_RANGE_HPP
#define XSTD_BITS_DETAIL_CONTAINER_COMPATIBLE_RANGE_HPP

#include <concepts> // convertible_to
#include <ranges>   // input_range, range_reference_t

namespace xstd::bits::detail {

// [container.intro.reqmts]'s container-compatible-range: an input range whose references convert to T.
template<class R, class T>
concept container_compatible_range = std::ranges::input_range<R> and std::convertible_to<std::ranges::range_reference_t<R>, T>;

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_CONTAINER_COMPATIBLE_RANGE_HPP
