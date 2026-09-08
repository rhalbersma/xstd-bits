//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_FORMAT_HPP
#define XSTD_BITS_FORMAT_HPP

#include <xstd/bits/bit_proxy.hpp>  // bit_sequence_reference, bit_set_reference
#include <cstddef>                  // size_t
#include <format>                   // formatter

// std::format over the containers, which needs nothing said about the containers themselves. [design.md#formatting-the-proxies]
//
// Every owner and view here is already a range, so [format.range.formatter] would format it -- except that the
// range formatter requires formattable<range_reference_t<R>>, and a reference of ours is a proxy. So the two
// proxies are what get a formatter, and every container over them follows: bit_set, bit_static_set, bit_vector,
// bit_array, the views, the windows, and the inplace column with them.
//
// Which is the shape the proxies already had for fmt. format_as is fmt's generic per-type hook -- define it for
// one type and every range over that type formats -- so the proxies carry it and no container does. This is the
// standard's hook for the same job, used the same way: one per proxy, containers for free.
//
// The readings then format in their own vocabulary without being asked to. [format.range.fmtkind] picks
// range_format::set for a range with a key_type and range_format::sequence otherwise, so the set reading prints
// {1, 3, 5} and the sequence reading [false, true, false, false] -- the same split fmt's format_as arrives at,
// reached through the standard's own machinery. [design.md#two-readings-disagree]
//
// Deriving from the underlying formatter rather than writing parse() is what keeps the whole format spec: a width,
// a fill, {:#x} on a position and {:d} on a bool all work, and the nested spec a range formatter forwards
// ({::#x}) reaches them.
//
// Not in <xstd/bits.hpp>: the umbrella keeps <format> off every consumer path for the same reason it keeps the
// ext/ adaptors and Boost off it. A consumer who formats says so by including this header.

// [namespace.std]/2 allows a specialization of a standard library template for a program-defined type, which is
// what these two are and all they are. clang-tidy 22 and 23 read the qualified definition as modifying namespace
// std anyway; 24 no longer does. [design.md#clang-tidy-false-positives]
template<class Bits, class Traits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::bit_set_reference<Bits, Traits>, CharT>
:
        std::formatter<std::size_t, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::bit_set_reference<Bits, Traits> ref, Context& ctx) const
        {
                return std::formatter<std::size_t, CharT>::format(static_cast<std::size_t>(ref), ctx);
        }
};

template<class Bits, class Traits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::bit_sequence_reference<Bits, Traits>, CharT>
:
        std::formatter<bool, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::bit_sequence_reference<Bits, Traits> ref, Context& ctx) const
        {
                return std::formatter<bool, CharT>::format(static_cast<bool>(ref), ctx);
        }
};

#endif  // XSTD_BITS_FORMAT_HPP
