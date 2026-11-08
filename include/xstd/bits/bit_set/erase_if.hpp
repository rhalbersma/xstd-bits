//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_ERASE_IF_HPP
#define XSTD_BITS_BIT_SET_ERASE_IF_HPP

#include <xstd/bits/bit_set/detail/erase_if.hpp> // IWYU pragma: export; erase_if

namespace xstd {

// [set.erasure]'s std::erase_if, spelled with xstd:: as a port spells it; ADL finds it too.
using bits::detail::erase_if;

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_ERASE_IF_HPP
