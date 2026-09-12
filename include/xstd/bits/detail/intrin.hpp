//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_INTRIN_HPP
#define XSTD_BITS_DETAIL_INTRIN_HPP

#include <xstd/ints/bit.hpp>                       // countl_zero, countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

// The seam, now closed on the xstd side. [design.md#uint128-support]
namespace xstd::detail::bits {

[[nodiscard]] constexpr auto countl_zero(xstd::unsigned_integer auto block) noexcept
        -> std::size_t
{
        return static_cast<std::size_t>(xstd::countl_zero(block));
}

[[nodiscard]] constexpr auto countr_zero(xstd::unsigned_integer auto block) noexcept
        -> std::size_t
{
        return static_cast<std::size_t>(xstd::countr_zero(block));
}

[[nodiscard]] constexpr auto popcount(xstd::unsigned_integer auto block) noexcept
        -> std::size_t
{
        return static_cast<std::size_t>(xstd::popcount(block));
}

}       // namespace xstd::detail::bits

#endif // XSTD_BITS_DETAIL_INTRIN_HPP
