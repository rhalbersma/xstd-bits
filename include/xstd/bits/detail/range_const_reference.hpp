//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
#define XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP

#include <version>                                               // IWYU pragma: keep; __cpp_lib_ranges_as_const
#include <xstd/bits/detail/range_const_reference_fallback.hpp>   // fallback::range_const_reference_t
#include <ranges>                                                // range, range_const_reference_t

namespace xstd::detail::bits {

// The standard's where the library has it, ours where it does not. libc++ has implemented P2278R4 on no branch, trunk included: __cpp_lib_ranges_as_const is still a commented-out line in its <version>, and neither as_const_view.h nor const_access.h exists, so a third of the matrix needs the fallback beside this. The two arms are the same type rather than two contracts, the fallback being the paper's own formula and TheConstReferenceIsP2278s asserting the agreement wherever both exist. [design.md#the-const-reference]
#ifdef __cpp_lib_ranges_as_const
template<std::ranges::range R>
using range_const_reference_t = std::ranges::range_const_reference_t<R>;
#else
template<std::ranges::range R>
using range_const_reference_t = fallback::range_const_reference_t<R>;
#endif

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
