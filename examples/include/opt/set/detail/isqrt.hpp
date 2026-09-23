//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef OPT_SET_DETAIL_ISQRT_HPP
#define OPT_SET_DETAIL_ISQRT_HPP

#include <cstddef> // size_t

namespace opt::detail {

// Newton on x * x - n, exact in size_t: floor division descends to floor(sqrt(n)), and n < 2 needs no guard.
constexpr auto isqrt(std::size_t n) noexcept
        -> std::size_t
{
        auto x = n;
        auto y = (x + 1UZ) / 2UZ;
        while (y < x) {
                x = y;
                y = (x + (n / x)) / 2UZ;
        }
        return x;
}

} // namespace opt::detail

#endif // OPT_SET_DETAIL_ISQRT_HPP
