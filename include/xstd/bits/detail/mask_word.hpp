//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_MASK_WORD_HPP
#define XSTD_BITS_DETAIL_MASK_WORD_HPP

#include <xstd/bits/detail/intrin.hpp>             // popcount
#include <xstd/bits/detail/shift.hpp>              // shl, shr
#include <xstd/ints/concepts/bit_mask.hpp>         // bit_mask
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits/numeric_limits.hpp>     // numeric_limits
#include <algorithm>                               // min
#include <bitset>                                  // bitset
#include <cstddef>                                 // size_t
#include <ranges>                                  // iota
#include <type_traits>                             // is_enum_v, make_unsigned_t, type_identity, underlying_type_t
#include <utility>                                 // to_underlying

// The word a flag type converts with: [bitmask.types]'s three forms, an enumeration, an unsigned integer or a bitset.
namespace xstd::bits::detail {

template<class Mask>
constexpr bool is_bitset = false;

template<std::size_t M>
constexpr bool is_bitset<std::bitset<M>> = true;

// A bit mask whose positions can be read and written one by one, which the operators alone do not allow.
template<class Mask>
concept mask_word = xstd::bit_mask<Mask> and (std::is_enum_v<Mask> or xstd::unsigned_integer<Mask> or is_bitset<Mask>);

// The unsigned integer an enumeration's or an integer's positions are read in.
template<class Mask>
struct mask_integer : std::type_identity<Mask>
{};

template<class Mask>
        requires std::is_enum_v<Mask>
struct mask_integer<Mask> : std::type_identity<std::make_unsigned_t<std::underlying_type_t<Mask>>>
{};

template<class Mask>
using mask_integer_t = mask_integer<Mask>::type;

// How many positions the mask holds.
template<mask_word Mask>
[[nodiscard]] consteval auto mask_width() noexcept
        -> std::size_t
{
        if constexpr (is_bitset<Mask>) {
                return Mask().size();
        } else {
                return static_cast<std::size_t>(xstd::numeric_limits<mask_integer_t<Mask>>::digits);
        }
}

// The mask's positions below n, as a block; n is at most the block's width.
template<xstd::unsigned_integer Block, mask_word Mask>
[[nodiscard]] constexpr auto low_mask_bits(Mask mask, std::size_t n) noexcept
        -> Block
{
        if constexpr (is_bitset<Mask>) {
                auto bits = Block{};
                for (auto const i : std::views::iota(0UZ, std::min(n, mask.size()))) {
                        if (mask[i]) {
                                bits = static_cast<Block>(bits | shl(Block{1}, i));
                        }
                }
                return bits;
        } else {
                auto const below = n == static_cast<std::size_t>(xstd::numeric_limits<Block>::digits) ? static_cast<Block>(~Block{}) : static_cast<Block>(shl(Block{1}, n) - Block{1});
                if constexpr (std::is_enum_v<Mask>) {
                        return static_cast<Block>(static_cast<Block>(static_cast<mask_integer_t<Mask>>(std::to_underlying(mask))) & below);
                } else {
                        return static_cast<Block>(static_cast<Block>(mask) & below);
                }
        }
}

// Whether every position the mask has set lies below n.
template<xstd::unsigned_integer Block, mask_word Mask>
[[nodiscard]] constexpr auto mask_fits(Mask mask, std::size_t n) noexcept
        -> bool
{
        auto const bits = low_mask_bits<Block>(mask, n);
        if constexpr (is_bitset<Mask>) {
                return mask.count() == popcount(bits);
        } else if constexpr (std::is_enum_v<Mask>) {
                return static_cast<mask_integer_t<Mask>>(bits) == static_cast<mask_integer_t<Mask>>(std::to_underlying(mask));
        } else {
                return static_cast<Mask>(bits) == mask;
        }
}

// The block as the mask; the block has no position at or above the mask's width set.
template<mask_word Mask, xstd::unsigned_integer Block>
[[nodiscard]] constexpr auto to_mask(Block bits) noexcept
        -> Mask
{
        if constexpr (is_bitset<Mask>) {
                auto nrv = Mask();
                for (auto const i : std::views::iota(0UZ, std::min(nrv.size(), static_cast<std::size_t>(xstd::numeric_limits<Block>::digits)))) {
                        nrv[i] = (shr(bits, i) & Block{1}) != Block{};
                }
                return nrv;
        } else if constexpr (std::is_enum_v<Mask>) {
                return static_cast<Mask>(static_cast<std::underlying_type_t<Mask>>(bits));
        } else {
                return static_cast<Mask>(bits);
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_MASK_WORD_HPP
