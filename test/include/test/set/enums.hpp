//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_ENUMS_HPP
#define TEST_SET_ENUMS_HPP

#include <xstd/bits/bit_enum_traits.hpp> // bit_enum_traits, enum_traits
#include <array>                         // array
#include <cstdint>                       // int16_t, int8_t, uint8_t
#include <format>                        // format_to, formatter
#include <string_view>                   // string_view
#include <tuple>                         // tuple

// Enumerations keying a set, each with its values listed beside it as an enumeration's author would list them.
namespace test::set {

// Dense from 0.
enum class perm : std::uint8_t
{
        read,
        write,
        exec,
};

// Dense from 10.
enum class letter : std::uint8_t
{
        a = 10,
        b,
        c,
};

// Gaps between the values, and a value far above the rest.
enum class piece : std::uint8_t
{
        pawn   = 1,
        knight = 3,
        bishop = 4,
        rook   = 8,
        queen  = 9,
        king   = 100,
};

// Dense from a negative value, in a type wider than a byte.
enum class level : std::int16_t
{
        low = -300,
        mid,
        high,
};

// A negative value and a gap.
enum class sign : std::int8_t
{
        minus = -2,
        zero  = 0,
        plus  = 1,
};

// Five values in an 8-bit block, three of its bits padding.
enum class day : std::uint8_t
{
        mon,
        tue,
        wed,
        thu,
        fri,
};

// Eight values filling an 8-bit block exactly.
enum class wind : std::uint8_t
{
        n,
        ne,
        e,
        se,
        s,
        sw,
        w,
        nw,
};

// Nine values, one more than an 8-bit block holds.
enum class nine : std::uint8_t
{
        v0,
        v1,
        v2,
        v3,
        v4,
        v5,
        v6,
        v7,
        v8,
};

// No values listed, so no traits place it in bits.
enum class undeclared : std::uint8_t
{
        x,
};

using listed_enums = std::tuple<perm, letter, piece, level, sign, day, wind, nine>;

} // namespace test::set

template<>
struct xstd::enum_traits<test::set::perm>
{
        static constexpr std::array values = {test::set::perm::read, test::set::perm::write, test::set::perm::exec};
};

template<>
struct xstd::enum_traits<test::set::letter>
{
        static constexpr std::array values = {test::set::letter::a, test::set::letter::b, test::set::letter::c};
};

template<>
struct xstd::enum_traits<test::set::piece>
{
        static constexpr std::array values = {test::set::piece::pawn, test::set::piece::knight, test::set::piece::bishop, test::set::piece::rook, test::set::piece::queen, test::set::piece::king};
};

template<>
struct xstd::enum_traits<test::set::level>
{
        static constexpr std::array values = {test::set::level::low, test::set::level::mid, test::set::level::high};
};

template<>
struct xstd::enum_traits<test::set::sign>
{
        static constexpr std::array values = {test::set::sign::minus, test::set::sign::zero, test::set::sign::plus};
};

template<>
struct xstd::enum_traits<test::set::day>
{
        static constexpr std::array values = {test::set::day::mon, test::set::day::tue, test::set::day::wed, test::set::day::thu, test::set::day::fri};
};

template<>
struct xstd::enum_traits<test::set::wind>
{
        static constexpr std::array values = {test::set::wind::n, test::set::wind::ne, test::set::wind::e, test::set::wind::se, test::set::wind::s, test::set::wind::sw, test::set::wind::w, test::set::wind::nw};
};

template<>
struct xstd::enum_traits<test::set::nine>
{
        static constexpr std::array values = {test::set::nine::v0, test::set::nine::v1, test::set::nine::v2, test::set::nine::v3, test::set::nine::v4, test::set::nine::v5, test::set::nine::v6, test::set::nine::v7, test::set::nine::v8};
};

// A piece prints its name, so a set that formats its positions rather than its keys shows it.
template<class CharT>
struct std::formatter<test::set::piece, CharT> : std::formatter<std::string_view, CharT>
{
        static constexpr std::array<std::string_view, 6> names = {"pawn", "knight", "bishop", "rook", "queen", "king"};

        template<class Context>
        [[nodiscard]] auto format(test::set::piece key, Context& ctx) const
        {
                return std::format_to(ctx.out(), "{}", names[xstd::bit_enum_traits<test::set::piece>::to_index(key)]);
        }
};

#endif // TEST_SET_ENUMS_HPP
