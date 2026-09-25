//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_STORAGE_HPP
#define XSTD_BITS_BIT_STORAGE_HPP

#include <xstd/bits/detail/range_const_reference.hpp> // range_const_reference_t
#include <xstd/ints/concepts/unsigned_integer.hpp>    // unsigned_integer
#include <xstd/ints/limits.hpp>                       // numeric_limits
#include <array>                                      // array
#include <concepts>                                   // convertible_to, regular, same_as
#include <cstddef>                                    // size_t
#include <ranges>                                     // contiguous_range, end, range, range_reference_t, range_size_t, range_value_t, sized_range
#include <span>                                       // dynamic_extent, span
#include <type_traits>                                // remove_const_t

// What every container and view here presents a packed interface over: bits in contiguous unsigned words.
namespace xstd {

// One unsigned word, or a sized contiguous range of them that subscripts; const where a view only reads.
template<class Bits>
concept bit_storage =
        xstd::unsigned_integer<std::remove_const_t<Bits>> or
        (std::ranges::sized_range<Bits> and std::ranges::contiguous_range<Bits> and
         xstd::unsigned_integer<std::remove_const_t<std::ranges::range_value_t<Bits>>> and
         requires (Bits& bits, std::ranges::range_size_t<Bits> n) { bits[n]; });

// Bit storage a container can own: a value compared by its words, and read-only through a const object.
template<class Bits>
concept owned_bit_storage =
        bit_storage<Bits> and std::regular<Bits> and
        (xstd::unsigned_integer<Bits> or
         requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n) {
                 { bits[n] } -> std::same_as<std::ranges::range_reference_t<Bits>>;
                 // P2278R4's alias: a storage whose const subscript yields a writable reference is refused.
                 { cbits[n] } -> std::same_as<bits::detail::range_const_reference_t<Bits>>;
         });

// Owned words whose count changes at run time: what an owner of a run-time width grows and shrinks.
template<class Bits>
concept resizable_bit_storage =
        owned_bit_storage<Bits> and std::ranges::range<Bits> and
        requires (Bits& bits, Bits const& cbits, std::ranges::range_size_t<Bits> n, std::ranges::range_value_t<Bits> const* words) {
                bits.resize(n, *words);
                bits.push_back(*words);
                bits.insert(std::ranges::end(bits), words, words);
                bits.clear();
                { cbits.max_size() } -> std::convertible_to<std::ranges::range_size_t<Bits>>;
        };

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
