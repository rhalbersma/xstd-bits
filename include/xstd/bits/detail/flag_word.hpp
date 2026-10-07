//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_FLAG_WORD_HPP
#define XSTD_BITS_DETAIL_FLAG_WORD_HPP

#include <xstd/bits/bit/bit_convert.hpp> // bit_convert
#include <xstd/bits/bit_blocks.hpp>      // least_block_t, underlying_block_t
#include <bitset>                        // bitset
#include <cstddef>                       // size_t
#include <limits>                        // numeric_limits
#include <type_traits>                   // is_enum_v, type_identity
#include <utility>                       // to_underlying

// The one block a flag type's mask is read and written as: an enumeration's unsigned word, or a bitset's.
namespace xstd::bits::detail {

template<class Mask>
struct flag_word
{};

// Unsigned, so an enumerator on a signed type's sign bit is one bit like the others.
template<class Mask>
        requires requires { typename xstd::underlying_block_t<Mask>; }
struct flag_word<Mask> : std::type_identity<xstd::underlying_block_t<Mask>>
{};

// A bitset exactly as wide as a block, which bit_convert maps position for position.
template<std::size_t M>
        requires (M == static_cast<std::size_t>(std::numeric_limits<xstd::least_block_t<M>>::digits))
struct flag_word<std::bitset<M>> : std::type_identity<xstd::least_block_t<M>>
{};

template<class Mask>
using flag_word_t = flag_word<Mask>::type;

// A type whose positions fit one block and convert both ways: an enumeration, or a std::bitset as wide as a block.
template<class Mask>
concept flag_mask = requires { typename flag_word_t<Mask>; };

template<flag_mask Mask>
inline constexpr auto flag_width_v = static_cast<std::size_t>(std::numeric_limits<flag_word_t<Mask>>::digits);

template<flag_mask Mask>
[[nodiscard]] constexpr auto to_word(Mask const& mask) noexcept
        -> flag_word_t<Mask>
{
        if constexpr (std::is_enum_v<Mask>) {
                return static_cast<flag_word_t<Mask>>(std::to_underlying(mask));
        } else {
                return xstd::bit_convert<flag_word_t<Mask>>(mask);
        }
}

template<flag_mask Mask>
[[nodiscard]] constexpr auto from_word(flag_word_t<Mask> word) noexcept
        -> Mask
{
        if constexpr (std::is_enum_v<Mask>) {
                return static_cast<Mask>(word);
        } else {
                return xstd::bit_convert<Mask>(word);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_FLAG_WORD_HPP
