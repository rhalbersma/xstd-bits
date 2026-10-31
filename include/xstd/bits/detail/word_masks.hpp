//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_WORD_MASKS_HPP
#define XSTD_BITS_DETAIL_WORD_MASKS_HPP

#include <xstd/bits/detail/block_width.hpp>        // block_width
#include <xstd/bits/detail/intrinsic.hpp>          // popcount
#include <xstd/bits/detail/shift.hpp>              // shl
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint64_t
#include <ranges>                                  // iota
#include <span>                                    // span

namespace xstd::bits::detail {

// The masks a bit_gslice stores as 64-bit words, read a block at a time at any block width.
template<xstd::unsigned_integer Block>
class word_masks
{
        static constexpr auto digits = block_width<Block>;
        static constexpr auto word   = 64UZ;

        std::span<std::uint64_t const> m_words;
        std::size_t m_last;

public:
        // The blocks that hold the first width positions.
        [[nodiscard]] constexpr word_masks(std::span<std::uint64_t const> words, std::size_t width) noexcept
                : m_words(words)
                , m_last((width + digits - 1UZ) / digits)
        {}

        template<class F>
        constexpr auto for_each(F f) const noexcept
                -> void
        {
                for (auto const k : std::views::iota(0UZ, m_last)) {
                        auto const pos = k * digits;
                        if constexpr (digits <= word) {
                                // A block of at most a word lies inside one word, its width being a power of two.
                                auto const mask = static_cast<Block>(m_words[pos / word] >> (pos % word));
                                f(k, mask, detail::popcount(mask));
                        } else {
                                // A wider block spans words, the last of which may lie past width.
                                auto mask = Block{};
                                for (auto const i : std::views::iota(0UZ, digits / word)) {
                                        auto const index = (pos / word) + i;
                                        auto const bits  = index < m_words.size() ? static_cast<Block>(m_words[index]) : Block{};
                                        mask             = static_cast<Block>(mask | shl(bits, i * word));
                                }
                                f(k, mask, detail::popcount(mask));
                        }
                }
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_WORD_MASKS_HPP
