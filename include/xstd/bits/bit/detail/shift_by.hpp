//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_DETAIL_SHIFT_BY_HPP
#define XSTD_BITS_BIT_DETAIL_SHIFT_BY_HPP

#include <xstd/bits/detail/ownership.hpp>       // owner, reads_keys
#include <xstd/ints/concepts/signed_integer.hpp> // signed_integer
#include <xstd/ints/cstdlib/unsigned_abs.hpp>    // unsigned_abs
#include <xstd/ints/limits/numeric_limits.hpp>   // numeric_limits
#include <cstddef>                               // size_t
#include <limits>                                // numeric_limits

namespace xstd::bits::detail {

// A set that owns its keys and shifts them, which shl and shr take by value.
template<class X>
concept shiftable_set = owner<X> and reads_keys<X> and requires (X& x, std::size_t n) {
        x <<= n;
        x >>= n;
        x.clear();
};

// Every count in the domain: a negative one turns the direction, and one at or past a static width empties the set.
template<class X, class S>
[[nodiscard]] constexpr auto shift_by(X x, S s, bool leftward) noexcept(noexcept(x <<= std::size_t{}) and noexcept(x >>= std::size_t{}))
        -> X
{
        if constexpr (xstd::signed_integer<S>) {
                leftward = (leftward != (s < S{0}));
        }
        auto const magnitude = xstd::unsigned_abs(s);
        using magnitude_type = decltype(magnitude);
        // Saturated at the top of size_t: a dynamic width empties past its max_size(), which is no more than that.
        auto n = std::numeric_limits<std::size_t>::max();
        if constexpr (xstd::numeric_limits<magnitude_type>::digits > std::numeric_limits<std::size_t>::digits) {
                if (magnitude <= static_cast<magnitude_type>(n)) {
                        n = static_cast<std::size_t>(magnitude);
                }
        } else {
                n = static_cast<std::size_t>(magnitude);
        }
        if constexpr (X::has_static_width) {
                if (n >= x.max_size()) {
                        x.clear();
                        return x;
                }
        }
        if (leftward) {
                x <<= n;
        } else {
                x >>= n;
        }
        return x;
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_DETAIL_SHIFT_BY_HPP
