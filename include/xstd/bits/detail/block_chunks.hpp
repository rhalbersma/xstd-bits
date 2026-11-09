//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BLOCK_CHUNKS_HPP
#define XSTD_BITS_DETAIL_BLOCK_CHUNKS_HPP

#include <algorithm> // min
#include <cstddef>   // size_t

namespace xstd::bits::detail {

// Whether pred(k, count) holds for a chunk of [n, n + len), lowest first: a head to a multiple of Digits, whole blocks.
template<std::size_t Digits, class Pred>
constexpr auto find_chunk(std::size_t n, std::size_t len, Pred pred)
        -> bool
{
        auto count = std::ranges::min(len, Digits - (n % Digits));
        for (auto k = 0UZ; k < len; k += count, count = std::ranges::min(Digits, len - k)) {
                if (pred(k, count)) {
                        return true;
                }
        }
        return false;
}

// The mirror, highest first: what lies above the last boundary, whole blocks, then the head below the first.
template<std::size_t Digits, class Pred>
constexpr auto find_chunk_down(std::size_t n, std::size_t len, Pred pred)
        -> bool
{
        if (len == 0UZ) {
                return false;
        }
        auto count = std::ranges::min(len, ((n + len - 1UZ) % Digits) + 1UZ);
        for (auto end = len; end != 0UZ; end -= count, count = std::ranges::min(Digits, end)) {
                if (pred(end - count, count)) {
                        return true;
                }
        }
        return false;
}

// f(k, count) for every chunk find_chunk walks, lowest first.
template<std::size_t Digits, class F>
constexpr auto for_each_chunk(std::size_t n, std::size_t len, F f)
        -> void
{
        find_chunk<Digits>(n, len, [&](std::size_t k, std::size_t count) -> bool {
                f(k, count);
                return false;
        });
}

// f(k, count) for every chunk find_chunk_down walks, highest first.
template<std::size_t Digits, class F>
constexpr auto for_each_chunk_down(std::size_t n, std::size_t len, F f)
        -> void
{
        find_chunk_down<Digits>(n, len, [&](std::size_t k, std::size_t count) -> bool {
                f(k, count);
                return false;
        });
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BLOCK_CHUNKS_HPP
