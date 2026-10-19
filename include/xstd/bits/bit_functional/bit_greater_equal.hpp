//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_GREATER_EQUAL_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_GREATER_EQUAL_HPP

#include <xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp> // bit_lexicographical_compare_three_way
#include <compare>                                                           // is_gteq

namespace xstd {

// std::greater_equal<> through bit_lexicographical_compare_three_way, over the pairs bit_equal_to takes.
struct bit_greater_equal
{
        using is_transparent = void;

        template<class T, class U>
                requires requires (T const& x, U const& y) { xstd::ranges::bit_lexicographical_compare_three_way(x, y); }
        [[nodiscard]] constexpr auto operator()(T const& x, U const& y) const noexcept
                -> bool
        {
                return std::is_gteq(xstd::ranges::bit_lexicographical_compare_three_way(x, y));
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_GREATER_EQUAL_HPP
