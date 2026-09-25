//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STORAGE_HPP
#define XSTD_BITS_BIT_STORAGE_HPP

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <array>                                   // array
#include <cstddef>                                 // size_t
#include <ranges>                                  // contiguous_range, range_size_t, range_value_t, sized_range
#include <span>                                    // dynamic_extent, span
#include <type_traits>                             // remove_const_t

// What every container and view here presents a packed interface over: bits in contiguous unsigned words.
namespace xstd {

// One unsigned word, or a sized contiguous range of them that subscripts; const where a view only reads.
template<class Bits>
concept bit_storage =
        xstd::unsigned_integer<std::remove_const_t<Bits>> or
        (std::ranges::sized_range<Bits> and std::ranges::contiguous_range<Bits> and
         xstd::unsigned_integer<std::remove_const_t<std::ranges::range_value_t<Bits>>> and
         requires (Bits& bits, std::ranges::range_size_t<Bits> n) { bits[n]; });

// The width bit storage names by its type: every bit of a word or of a fixed number of words, else dynamic_extent.
template<bit_storage Bits>
inline constexpr std::size_t bit_storage_extent_v = std::dynamic_extent;

template<bit_storage Bits>
        requires xstd::unsigned_integer<std::remove_const_t<Bits>>
inline constexpr std::size_t bit_storage_extent_v<Bits> = static_cast<std::size_t>(xstd::numeric_limits<std::remove_const_t<Bits>>::digits);

template<xstd::unsigned_integer Word, std::size_t K>
inline constexpr std::size_t bit_storage_extent_v<std::array<Word, K>> = K * bit_storage_extent_v<Word>;

template<xstd::unsigned_integer Word, std::size_t K>
inline constexpr std::size_t bit_storage_extent_v<std::array<Word, K> const> = bit_storage_extent_v<std::array<Word, K>>;

template<class Word, std::size_t E>
        requires xstd::unsigned_integer<std::remove_const_t<Word>> and (E != std::dynamic_extent)
inline constexpr std::size_t bit_storage_extent_v<std::span<Word, E>> = E * bit_storage_extent_v<Word>;

} // namespace xstd

#endif // XSTD_BITS_BIT_STORAGE_HPP
