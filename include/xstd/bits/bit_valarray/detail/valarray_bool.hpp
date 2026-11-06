//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_BOOL_HPP
#define XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_BOOL_HPP

#include <xstd/bits/detail/bit_convertible.hpp> // bit_convert_source, bit_source, bit_target
#include <xstd/bits/detail/bit_layout.hpp>      // bits_per_byte, block_byte, or_block_byte, value_bytes
#include <algorithm>                            // count, min
#include <cstddef>                              // size_t
#include <ranges>                               // iota
#include <span>                                 // span
#include <valarray>                             // begin, end, valarray
#include <vector>                               // vector

// std::valarray<bool>'s side of bit_convert, a bool per position: the mask [valarray.sub] subscripts a valarray by.
namespace xstd::bits::detail {

template<>
struct bit_source<std::valarray<bool>>
{
        [[nodiscard]] static auto width(std::valarray<bool> const& from) noexcept
                -> std::size_t
        {
                return from.size();
        }

        [[nodiscard]] static auto count(std::valarray<bool> const& from) noexcept
                -> std::size_t
        {
                return static_cast<std::size_t>(std::count(std::begin(from), std::end(from), true));
        }

        // Packed a byte at a time into blocks that start clear; what lies past the target drops.
        template<class U>
        static auto copy(std::valarray<bool> const& from, std::span<U> dst) noexcept
                -> void
        {
                auto const n = std::ranges::min((from.size() + bits_per_byte - 1UZ) / bits_per_byte, value_bytes(dst));
                for (auto const j : std::views::iota(0UZ, n)) {
                        auto byte = 0U;
                        for (auto const k : std::views::iota(0UZ, std::ranges::min(bits_per_byte, from.size() - (j * bits_per_byte)))) {
                                byte |= static_cast<unsigned>(from[(j * bits_per_byte) + k]) << k;
                        }
                        or_block_byte(dst, j, static_cast<unsigned char>(byte));
                }
        }
};

// A valarray of the source's width, unpacked from its bytes: every source copies into bytes, a window included.
template<>
struct bit_target<std::valarray<bool>>
{
        template<bit_convert_source From>
        [[nodiscard]] static auto convert(From const& from)
                -> std::valarray<bool>
        {
                using source     = bit_source<From>;
                auto const width = source::width(from);
                auto bytes       = std::vector<unsigned char>((width + bits_per_byte - 1UZ) / bits_per_byte);
                source::copy(from, std::span(bytes));
                auto to = std::valarray<bool>(width);
                for (auto const i : std::views::iota(0UZ, width)) {
                        to[i] = ((static_cast<unsigned>(block_byte(bytes, i / bits_per_byte)) >> (i % bits_per_byte)) & 1U) != 0U;
                }
                return to;
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_BOOL_HPP
