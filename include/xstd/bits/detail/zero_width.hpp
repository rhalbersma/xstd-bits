//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ZERO_WIDTH_HPP
#define XSTD_BITS_DETAIL_ZERO_WIDTH_HPP

#include <type_traits> // remove_const_t

namespace xstd::detail::bits {

// A zero width answers zero to every question, and says so here, before a walk is instantiated for it. It is asked of the storage now rather than of a trait, and it is not decoration: the exclusive scans take a position as a precondition, a zero width has none to give, and they assert there. Every caller that steps must test this first, which is what the trait's scans were doing before the storage was ever reached.
template<class Bits>
constexpr bool zero_width = std::remove_const_t<Bits>::extent == 0UZ;

} // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_ZERO_WIDTH_HPP
