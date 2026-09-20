//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ZERO_WIDTH_HPP
#define XSTD_BITS_DETAIL_ZERO_WIDTH_HPP

#include <type_traits> // remove_const_t

namespace xstd::detail::bits {

// A zero width answers zero to every question: the exclusive scans need a position it has none to give.
template<class Bits>
constexpr bool zero_width = std::remove_const_t<Bits>::extent == 0UZ;

} // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_ZERO_WIDTH_HPP
