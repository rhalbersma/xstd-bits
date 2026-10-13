//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_EQUAL_TO_HPP
#define XSTD_BITS_BIT_EQUAL_TO_HPP

#include <xstd/bits/bit_algorithm/bit_equal.hpp> // bit_equal

// The equality bit_hasher hashes for: equal bits whatever blocks or storage hold them, so lookups cross containers.
namespace xstd {

// std::equal_to<> through xstd::bit_equal, constrained on the call itself so the two cannot drift: transparent.
struct bit_equal_to
{
        using is_transparent = void;

        template<class T, class U>
                requires requires (T const& x, U const& y) { xstd::bit_equal(x, y); }
        [[nodiscard]] constexpr auto operator()(T const& x, U const& y) const noexcept
                -> bool
        {
                return xstd::bit_equal(x, y);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_EQUAL_TO_HPP
