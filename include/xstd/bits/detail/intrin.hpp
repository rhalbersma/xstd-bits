//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_INTRIN_HPP
#define XSTD_BITS_DETAIL_INTRIN_HPP

#include <xstd/ints/bit.hpp>                       // countl_zero, countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

// The seam, now closed on the xstd side. It was always constrained on xstd::unsigned_integer, an OPEN concept
// that a 128-bit integer class can join; it forwarded to <bit>, whose domain is std::unsigned_integral, a CLOSED
// one that no class can. Every such Block therefore satisfied the interface and then failed inside the body.
// xstd::popcount and friends are that same domain plus an overload per integer class, so this is the change of
// body the seam was left open for: MSVC's std::_Unsigned128, absl::uint128 and boost::int128::uint128 become
// usable Blocks, and the built-ins keep forwarding to <bit> unchanged. [design.md#uint128-support]
//
// The qualified call binds at THIS definition, so a Block's overload must be declared before this header is
// parsed. The overloads for the built-ins arrive with <xstd/ints/bit.hpp> above; an integer class carries its
// own beside the header that introduces it, so a translation unit reaching for one includes that ext header
// first. The test tree does it in test/block_types.hpp, ahead of every container header.
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
