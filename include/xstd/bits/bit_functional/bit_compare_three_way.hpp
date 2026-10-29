//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FUNCTIONAL_BIT_COMPARE_THREE_WAY_HPP
#define XSTD_BITS_BIT_FUNCTIONAL_BIT_COMPARE_THREE_WAY_HPP

#include <xstd/bits/bit_algorithm/bit_lexicographical_compare_three_way.hpp> // bit_lexicographical_compare_three_way
#include <compare>                                                           // strong_ordering

namespace xstd {

// std::compare_three_way through bit_lexicographical_compare_three_way, over the pairs bit_equal_to takes.
struct bit_compare_three_way
{
        using is_transparent = void;

        template<class T, class U>
                requires requires (T const& x, U const& y) { xstd::ranges::bit_lexicographical_compare_three_way(x, y); }
        [[nodiscard]] constexpr auto operator()(T const& x, U const& y) const noexcept
                -> std::strong_ordering
        {
                return xstd::ranges::bit_lexicographical_compare_three_way(x, y);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FUNCTIONAL_BIT_COMPARE_THREE_WAY_HPP
