//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_NOT_EQUAL_TO_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_NOT_EQUAL_TO_HPP

#include <xstd/bits/bit_algorithm/bit_equal.hpp> // bit_equal

namespace xstd {

// std::not_equal_to<> through xstd::bit_equal, over the pairs bit_equal_to takes: transparent.
struct bit_not_equal_to
{
        using is_transparent = void;

        template<class T, class U>
                requires requires (T const& x, U const& y) { xstd::bit_equal(x, y); }
        [[nodiscard]] constexpr auto operator()(T const& x, U const& y) const noexcept
                -> bool
        {
                return not xstd::bit_equal(x, y);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_NOT_EQUAL_TO_HPP
