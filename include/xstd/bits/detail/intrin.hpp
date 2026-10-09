//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_INTRIN_HPP
#define XSTD_BITS_DETAIL_INTRIN_HPP

#include <xstd/bits/detail/bit_layout.hpp>         // bits_per_byte
#include <xstd/ints/bit.hpp>                       // countl_zero, countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <algorithm>                               // copy
#include <array>                                   // array
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint64_t
#include <limits>                                  // numeric_limits
#include <ranges>                                  // iota
#include <span>                                    // span

#ifdef __AVX512BW__

#include <immintrin.h> // _mm512_and_si512, _mm512_movm_epi8, _mm512_set1_epi8, _mm512_storeu_si512

#elifdef __BMI2__

#include <immintrin.h> // _pdep_u64
#include <cstring>     // memcpy

#endif

namespace xstd::bits::detail {

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

inline constexpr auto bools_per_block = static_cast<std::size_t>(std::numeric_limits<std::uint64_t>::digits);

using byte_bools_table = std::array<std::array<unsigned char, bits_per_byte>, std::size_t{1} << bits_per_byte>;

// Every byte's bits as the bytes 0 and 1 a bool holds, least significant first: the portable and constant expansion.
inline constexpr auto byte_bools = [] -> byte_bools_table {
        auto table = byte_bools_table();
        for (auto const byte : std::views::iota(0UZ, table.size())) {
                for (auto const i : std::views::iota(0UZ, bits_per_byte)) {
                        table[byte][i] = static_cast<unsigned char>((byte >> i) & 1UZ);
                }
        }
        return table;
}();

constexpr auto expand_block_by_table(std::uint64_t block, std::span<unsigned char, bools_per_block> bools) noexcept
        -> void
{
        for (auto const i : std::views::iota(0UZ, bools_per_block / bits_per_byte)) {
                auto const byte = static_cast<unsigned char>(block >> (bits_per_byte * i));
                std::ranges::copy(byte_bools[byte], bools.subspan(bits_per_byte * i, bits_per_byte).begin());
        }
}

// A 64-bit block's bits as 64 bools, bit i in byte i: one instruction per byte under BMI2, one in all under AVX-512BW.
constexpr auto expand_block(std::uint64_t block, std::span<unsigned char, bools_per_block> bools) noexcept
        -> void
{
#if defined(__AVX512BW__) || defined(__BMI2__)
        if !consteval {
#ifdef __AVX512BW__
                _mm512_storeu_si512(bools.data(), _mm512_and_si512(_mm512_movm_epi8(block), _mm512_set1_epi8(1)));
#else
                // The low bit of each byte: pdep deposits the next source bit at each bit of the mask.
                constexpr auto low_bits = std::uint64_t{0x0101'0101'0101'0101};
                for (auto const i : std::views::iota(0UZ, bools_per_block / bits_per_byte)) {
                        auto const spread = static_cast<std::uint64_t>(_pdep_u64(block >> (bits_per_byte * i), low_bits));
                        std::memcpy(bools.subspan(bits_per_byte * i, bits_per_byte).data(), &spread, bits_per_byte);
                }
#endif
                return;
        }
#endif
        expand_block_by_table(block, bools);
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_INTRIN_HPP
