//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SHL_HPP
#define XSTD_BITS_BIT_SHL_HPP

#include <xstd/bits/bit/detail/shift_by.hpp> // shift_by, shiftable_set
#include <xstd/ints/concepts/integer.hpp>    // integer

// The set's keys shifted by any count of any integer type, total as the integer shl is beside a set's own operator.
namespace xstd {

// Each key moved s positions up, a negative s moving them down; a key moved past the width is dropped.
template<class X, integer S>
        requires bits::detail::shiftable_set<X>
[[nodiscard]] constexpr auto shl(X x, S s) noexcept(noexcept(bits::detail::shift_by(x, s, true)))
        -> X
{
        return bits::detail::shift_by(x, s, true);
}

} // namespace xstd

#endif // XSTD_BITS_BIT_SHL_HPP
