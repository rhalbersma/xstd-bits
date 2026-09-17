//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BYTEWISE_BITSET_HPP
#define XSTD_BITS_DETAIL_BYTEWISE_BITSET_HPP

#include <array>       // array
#include <bit>         // bit_cast
#include <bitset>      // bitset
#include <cstddef>     // byte, size_t, to_integer
#include <limits>      // numeric_limits
#include <type_traits> // has_unique_object_representations_v

namespace xstd::detail::bits {

// A std::bitset's positions as BYTES and back, byte j holding the positions [8j, 8j + 8) least significant bit
// first -- which is what every contiguous bit container lays them out as, whatever its block width, so this is the
// one currency our blocks and theirs can be exchanged in without walking positions.
template<std::size_t N>
inline constexpr auto bitset_byte_count = (N + 7UZ) / 8UZ;

// Two paths, and the narrow one is the STANDARD one: at a width an unsigned long long can hold, to_ullong and the
// constructor taking one are the door the standard itself provides, so nothing about any implementation's layout
// is assumed, asserted or even asked. The overflow_error to_ullong could throw is out of reach here, every value
// of such a width fitting -- the same argument design.md#... already makes for block access on libc++.
//
// That the narrow path exists is what keeps the wide one simple. The MSVC STL picks unsigned long for its word
// type below thirty-three bits and unsigned long long above (stl/inc/bitset:86), so sizeof(std::bitset<32>) is
// four there and eight everywhere else -- and every width where that is true is a width the narrow path takes,
// so the wide path below sees eight-byte words on all three standard libraries and needs no case for it.
template<std::size_t N>
[[nodiscard]] constexpr auto bitset_object_bytes(std::bitset<N> const& bs) noexcept
        -> std::array<std::byte, sizeof(std::bitset<N>)>
{
        return std::bit_cast<std::array<std::byte, sizeof(std::bitset<N>)>>(bs);
}

template<std::size_t N>
inline constexpr auto fits_one_word = N <= static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);

template<std::size_t N>
[[nodiscard]] constexpr auto bitset_bytes(std::bitset<N> const& bs) noexcept
        -> std::array<std::byte, bitset_byte_count<N>>
{
        auto bytes = std::array<std::byte, bitset_byte_count<N>>();

        // A zero width carries no position and therefore has no byte to carry one in. Said with if constexpr rather
        // than by letting the loops below run zero times: that instantiation would then hold a loop no width can
        // enter, which is a line no test reaches and a branch slot nothing can take -- both counted per
        // instantiation ([per-instantiation-slots](#per-instantiation-slots)), and measured here before it
        // ever reached CI.
        if constexpr (bitset_byte_count<N> > 0UZ) {
                if constexpr (fits_one_word<N>) {
                        auto const value = bs.to_ullong();
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = static_cast<std::byte>(static_cast<unsigned char>(value >> (8UZ * j)));
                        }
                } else {
                        auto const object = bitset_object_bytes(bs);
                        for (auto j = 0UZ; j < bytes.size(); ++j) {
                                bytes[j] = object[j];
                        }
                }
        }
        return bytes;
}

template<std::size_t N>
[[nodiscard]] constexpr auto bytes_bitset(std::array<std::byte, bitset_byte_count<N>> const& bytes) noexcept
        -> std::bitset<N>
{
        // Zero width first, for the reason the other direction gives.
        if constexpr (bitset_byte_count<N> == 0UZ) {
                return std::bitset<N>();
        } else if constexpr (fits_one_word<N>) {
                auto value = 0ULL;
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        value |= static_cast<unsigned long long>(std::to_integer<unsigned char>(bytes[j])) << (8UZ * j);
                }
                return std::bitset<N>(value);
        } else {
                auto object = std::array<std::byte, sizeof(std::bitset<N>)>();
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        object[j] = bytes[j];
                }
                return std::bit_cast<std::bitset<N>>(object);
        }
}

// Proves the wide path for ONE width, position by position, including that every OTHER byte stays clear. That
// second half is what makes it a proof rather than a spot check: it refuses a reordered word, a reversed bit
// order, a trailing word the implementation does not keep clean, and -- because bit_cast hands back the object
// representation rather than the value -- a big-endian target, where bit n of a word lands at the far end of it.
// So the endianness guard is this probe, and there is no separate one that could fall out of step with it.
template<std::size_t M>
        requires (not fits_one_word<M>)
[[nodiscard]] constexpr auto bitset_layout_holds() noexcept
        -> bool
{
        for (auto i = 0UZ; i < M; ++i) {
                auto b = std::bitset<M>();
                b.set(i);
                auto const object = bitset_object_bytes(b);
                for (auto j = 0UZ; j < object.size(); ++j) {
                        if (object[j] != (j == i / 8UZ ? static_cast<std::byte>(1U << (i % 8UZ)) : std::byte{})) {
                                return false;
                        }
                }
        }
        return true;
}

// FIXED widths, never the caller's: the layout is a property of the standard library, so proving it once costs the
// same whatever is being converted. Probing the caller's N instead is quadratic in it -- every position against
// every byte -- which is nothing at two hundred positions and a hung compiler at a million. All four are above
// what one word holds, because that is the only range the wide path is ever asked about: sixty-five crosses into a
// second word by one position, seventy-two fills that word's first byte exactly, and two hundred leaves a tail.
inline constexpr auto bitset_layout_verified =
        bitset_layout_holds< 65UZ>() and
        bitset_layout_holds< 72UZ>() and
        bitset_layout_holds<128UZ>() and
        bitset_layout_holds<200UZ>()
;

// A width one word holds needs nothing of any of this and says so first. Above that,
// has_unique_object_representations_v comes before the probe, and the order is load-bearing: it is exactly the
// precondition for a meaningful bit_cast -- trivially copyable AND unpadded -- and a padded std::bitset would make
// the probe ill-formed rather than false, bit_cast over padding being no constant expression. Atomic constraints
// are checked in order, so asking this first turns that hard error into an ordinary unsatisfied concept.
//
// The size window then bounds the object at its own positions plus at most one spare word, which catches a layout
// carrying a slot the fixed probes never reach: a spare word would give std::bitset<128> a hundred and ninety-two
// bits, and 192 < 192 is false.
template<std::size_t N>
concept bytewise_bitset =
        fits_one_word<N> or (
                std::has_unique_object_representations_v<std::bitset<N>> and
                sizeof(std::bitset<N>) * 8UZ >= N and
                sizeof(std::bitset<N>) * 8UZ <  N + 64UZ and
                bitset_layout_verified
        )
;

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_BYTEWISE_BITSET_HPP
