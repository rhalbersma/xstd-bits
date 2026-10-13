//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_FLAG_BLOCK_HPP
#define XSTD_BITS_DETAIL_FLAG_BLOCK_HPP

#include <xstd/bits/bit/bit_convert.hpp>                // bit_convert
#include <xstd/bits/bit_type_traits/bit_least.hpp>      // least_block_t
#include <xstd/bits/bit_type_traits/bit_underlying.hpp> // underlying_block_t
#include <xstd/bits/detail/ordinal.hpp>                 // ordinal_key, ordinal_t
#include <xstd/ints/limits/numeric_limits.hpp>          // numeric_limits
#include <bitset>                                       // bitset
#include <concepts>                                     // integral
#include <cstddef>                                      // size_t
#include <limits>                                       // numeric_limits
#include <type_traits>                                  // is_enum_v, type_identity

// The block a flag type's mask is read and written as: an enumeration's or an integer's unsigned type, or a bitset's.
namespace xstd::bits::detail {

template<class Mask>
struct flag_block
{};

// Unsigned, so a signed type's bits shift and count as an unsigned block's do, its sign bit no flag's.
template<class Mask>
        requires requires { typename xstd::underlying_block_t<Mask>; }
struct flag_block<Mask> : std::type_identity<xstd::underlying_block_t<Mask>>
{};

// A bitset exactly as wide as a block, which bit_convert maps position for position.
template<std::size_t N>
        requires (N == static_cast<std::size_t>(std::numeric_limits<xstd::least_block_t<N>>::digits))
struct flag_block<std::bitset<N>> : std::type_identity<xstd::least_block_t<N>>
{};

template<class Mask>
using flag_block_t = flag_block<Mask>::type;

// A type whose positions fit one block and convert both ways: an enumeration, an integer, or a block-wide std::bitset.
template<class Mask>
concept flag_mask = requires { typename flag_block_t<Mask>; };

// The integer a mask's width is counted in: an enumeration's underlying type, an integer itself, a bitset's block.
template<class Mask>
struct flag_word : std::type_identity<flag_block_t<Mask>>
{};

template<ordinal_key Mask>
struct flag_word<Mask> : std::type_identity<ordinal_t<Mask>>
{};

template<class Mask>
using flag_word_t = flag_word<Mask>::type;

// Every bit of the word but a sign bit, an integer's or a signed enumeration's alike, so every mask stays non-negative.
template<flag_mask Mask>
inline constexpr auto flag_width_v = static_cast<std::size_t>(xstd::numeric_limits<flag_word_t<Mask>>::digits);

// Not an enumeration without a fixed underlying type, whose values stop at the bit-field holding its enumerators.
template<class Mask>
concept full_range_mask = not std::is_enum_v<Mask> or requires { Mask{0}; };

// Only where every value of the word is one of the mask's: libc++'s regex flags would otherwise leave their range.
template<flag_mask Mask>
        requires full_range_mask<Mask>
inline constexpr auto default_flag_width_v = flag_width_v<Mask>;

// Flags by intent: | compiles, whatever it returns, so an enumerated type such as file_type is turned away.
template<class Mask>
concept flag_enumeration = std::is_enum_v<Mask> and requires (Mask a, Mask b) { a | b; };

template<flag_mask Mask>
[[nodiscard]] constexpr auto to_block(Mask const& mask) noexcept
        -> flag_block_t<Mask>
{
        if constexpr (std::is_enum_v<Mask> or std::integral<Mask>) {
                return static_cast<flag_block_t<Mask>>(mask);
        } else {
                return xstd::bit_convert<flag_block_t<Mask>>(mask);
        }
}

template<flag_mask Mask>
[[nodiscard]] constexpr auto from_block(flag_block_t<Mask> block) noexcept
        -> Mask
{
        if constexpr (std::is_enum_v<Mask> or std::integral<Mask>) {
                return static_cast<Mask>(block);
        } else {
                return xstd::bit_convert<Mask>(block);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_FLAG_BLOCK_HPP
