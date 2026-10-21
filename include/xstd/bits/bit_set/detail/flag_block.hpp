//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_DETAIL_FLAG_BLOCK_HPP
#define XSTD_BITS_BIT_SET_DETAIL_FLAG_BLOCK_HPP

#include <xstd/bits/bit_type_traits/bit_least.hpp>      // least_block_t
#include <xstd/bits/bit_type_traits/bit_underlying.hpp> // underlying_block_t
#include <xstd/bits/detail/bit_layout.hpp>              // bit_bytes, bits_per_byte, byte_count, bytes_bits, container_source
#include <xstd/bits/detail/intrinsic.hpp>               // countr_zero, popcount
#include <xstd/bits/detail/ordinal.hpp>                 // ordinal_t
#include <xstd/bits/detail/shift.hpp>                   // shl
#include <xstd/ints/concepts/integer.hpp>               // integer
#include <xstd/ints/concepts/unsigned_integer.hpp>      // unsigned_integer
#include <xstd/ints/limits/numeric_limits.hpp>          // numeric_limits
#include <xstd/ints/type_traits/make_unsigned.hpp>      // make_unsigned_t
#include <algorithm>                                    // ranges::find_if
#include <array>                                        // array
#include <bitset>                                       // bitset
#include <cstddef>                                      // size_t
#include <iterator>                                     // ranges::distance
#include <limits>                                       // numeric_limits
#include <ranges>                                       // views::iota
#include <type_traits>                                  // conditional_t, is_enum_v, type_identity

// The word a flag type's mask is read and written as: an enumeration's or an integer's unsigned type, or a bitset's.
namespace xstd::bits::detail {

template<class Mask>
struct flag_block
{};

// Unsigned, so a signed type's bits shift and count as an unsigned block's do, its sign bit no flag's.
template<class Mask>
        requires requires { typename xstd::underlying_block_t<Mask>; }
struct flag_block<Mask> : std::type_identity<xstd::underlying_block_t<Mask>>
{};

// A 128-bit integer where the dialect leaves it out of std::integral, as a strict -std=c++23 does.
template<class Mask>
        requires xstd::integer<Mask> and (not std::is_class_v<Mask>) and (not requires { typename xstd::underlying_block_t<Mask>; })
struct flag_block<Mask> : std::type_identity<xstd::make_unsigned_t<Mask>>
{};

// A bitset in the least block holding it, or past the widest block in an array of the least blocks holding it.
template<std::size_t M>
        requires (M > 0UZ)
struct flag_block<std::bitset<M>> : std::type_identity<std::conditional_t<(M <= static_cast<std::size_t>(std::numeric_limits<xstd::least_block_t<M>>::digits)), xstd::least_block_t<M>, std::array<xstd::least_block_t<M>, (M + std::numeric_limits<xstd::least_block_t<M>>::digits - 1UZ) / std::numeric_limits<xstd::least_block_t<M>>::digits>>>
{};

template<class Mask>
using flag_block_t = flag_block<Mask>::type;

// A type whose positions convert both ways with a word: an enumeration, an integer, or a std::bitset of any width.
template<class Mask>
concept flag_mask = requires { typename flag_block_t<Mask>; };

// Every bit of the word but a sign bit, an integer's or a signed enumeration's alike, so every mask stays non-negative.
template<flag_mask Mask>
inline constexpr auto flag_width_v = static_cast<std::size_t>(xstd::numeric_limits<ordinal_t<Mask>>::digits);

// A bitset's own width, its word holding nothing above it.
template<std::size_t M>
inline constexpr auto flag_width_v<std::bitset<M>> = M;

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

// A bitset's positions as bytes, least first, read by value where its object bytes are not in that order.
template<std::size_t M>
[[nodiscard]] constexpr auto bitset_bytes(std::bitset<M> const& mask) noexcept
        -> std::array<unsigned char, byte_count<M>>
{
        auto bytes = std::array<unsigned char, byte_count<M>>();
        for (auto const i : std::views::iota(0UZ, M)) {
                if (mask[i]) {
                        auto& byte = bytes[i / bits_per_byte];
                        byte       = static_cast<unsigned char>(static_cast<unsigned>(byte) | (1U << (i % bits_per_byte)));
                }
        }
        return bytes;
}

// Bytes of positions back into a bitset by value, position by position.
template<std::size_t M>
[[nodiscard]] constexpr auto bytes_bitset(std::array<unsigned char, byte_count<M>> const& bytes) noexcept
        -> std::bitset<M>
{
        auto mask = std::bitset<M>();
        for (auto const i : std::views::iota(0UZ, M)) {
                mask[i] = ((static_cast<unsigned>(bytes[i / bits_per_byte]) >> (i % bits_per_byte)) & 1U) != 0U;
        }
        return mask;
}

template<flag_mask Mask>
[[nodiscard]] constexpr auto to_block(Mask const& mask) noexcept
        -> flag_block_t<Mask>
{
        if constexpr (std::is_enum_v<Mask> or xstd::integer<Mask>) {
                return static_cast<flag_block_t<Mask>>(mask);
        } else if constexpr (container_source<Mask, flag_width_v<Mask>>) {
                return bytes_bits<flag_block_t<Mask>, flag_width_v<Mask>>(bit_bytes<flag_width_v<Mask>>(mask));
        } else {
                return bytes_bits<flag_block_t<Mask>, flag_width_v<Mask>>(bitset_bytes(mask));
        }
}

// The word holds no bit at or above the mask's width.
template<flag_mask Mask>
[[nodiscard]] constexpr auto from_block(flag_block_t<Mask> const& block) noexcept
        -> Mask
{
        if constexpr (std::is_enum_v<Mask> or xstd::integer<Mask>) {
                return static_cast<Mask>(block);
        } else if constexpr (container_source<Mask, flag_width_v<Mask>>) {
                return bytes_bits<Mask, flag_width_v<Mask>>(bit_bytes<flag_width_v<Mask>>(block));
        } else {
                return bytes_bitset<flag_width_v<Mask>>(bit_bytes<flag_width_v<Mask>>(block));
        }
}

// The bits set in a word: one block's, or the sum over an array of them.
template<class Word>
[[nodiscard]] constexpr auto word_count(Word const& word) noexcept
        -> std::size_t
{
        if constexpr (xstd::unsigned_integer<Word>) {
                return popcount(word);
        } else {
                auto count = 0UZ;
                for (auto const block : word) {
                        count += popcount(block);
                }
                return count;
        }
}

// The position of a word's lowest set bit, which it has.
template<class Word>
[[nodiscard]] constexpr auto word_countr_zero(Word const& word) noexcept
        -> std::size_t
{
        if constexpr (xstd::unsigned_integer<Word>) {
                return countr_zero(word);
        } else {
                using block_type = Word::value_type;
                auto const first = std::ranges::find_if(word, [](block_type block) noexcept -> bool { return block != block_type{}; });
                return (static_cast<std::size_t>(std::ranges::distance(word.begin(), first)) * static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits)) + countr_zero(*first);
        }
}

// The word with the bit at index alone.
template<class Word>
[[nodiscard]] constexpr auto word_bit(std::size_t index) noexcept
        -> Word
{
        if constexpr (xstd::unsigned_integer<Word>) {
                return shl(Word{1}, index);
        } else {
                using block_type     = Word::value_type;
                constexpr auto width = static_cast<std::size_t>(xstd::numeric_limits<block_type>::digits);
                auto word            = Word();
                word[index / width]  = shl(block_type{1}, index % width);
                return word;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_SET_DETAIL_FLAG_BLOCK_HPP
