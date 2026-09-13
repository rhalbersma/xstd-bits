//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
#define XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP

#include <version> // IWYU pragma: keep; __cpp_lib_ranges_as_const
#include <ranges>  // range, range_const_reference_t, range_reference_t

namespace xstd::detail::bits {

// P2278's alias where the library has it, the reference a const R iterates where it does not, so the concept below names one thing and this header is the only place the two spellings meet. libc++ has implemented the paper on no branch, trunk included: __cpp_lib_ranges_as_const is still commented out in its <version>, so the fallback carries every libc++ rung rather than being a legacy arm. [design.md#the-const-reference-shim]
//
// The two are not the same question, and the difference is a real one rather than a spelling: P2278 asks what R's own iterator yields once const-ified, the fallback what a const R iterates. They agree wherever const reaches the elements and part where it does not, so a shallow-const, span-like blocks type is admitted by the fallback and rejected by P2278. Every storage this library ships is deep-const, and TheConstReferenceShimAgreesWithP2278 pins that, so the divergence is real but unreachable from here. [design.md#the-const-reference-shim]
#ifdef __cpp_lib_ranges_as_const
template<std::ranges::range R>
using range_const_reference_t = std::ranges::range_const_reference_t<R>;
#else
template<std::ranges::range R>
using range_const_reference_t = std::ranges::range_reference_t<R const>;
#endif

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_RANGE_CONST_REFERENCE_HPP
