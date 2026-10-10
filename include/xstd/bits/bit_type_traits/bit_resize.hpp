//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_TYPE_TRAITS_BIT_RESIZE_HPP
#define XSTD_BITS_BIT_TYPE_TRAITS_BIT_RESIZE_HPP

#include <xstd/bits/detail/rebind.hpp> // rebind, resizable
#include <cstddef>                     // size_t

namespace xstd {

// The same bit container N bits wide, as std::simd's resize_t takes another width; a run-time width has none to change.
template<std::size_t N, class Owner>
        requires bits::detail::resizable<Owner>
using bit_resize = bits::detail::rebind<Owner>::template with_width<N>;

} // namespace xstd

#endif // XSTD_BITS_BIT_TYPE_TRAITS_BIT_RESIZE_HPP
