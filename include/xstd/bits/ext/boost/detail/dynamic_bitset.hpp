//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_EXT_BOOST_DETAIL_DYNAMIC_BITSET_HPP
#define XSTD_BITS_EXT_BOOST_DETAIL_DYNAMIC_BITSET_HPP

#include <xstd/bits/detail/bit_convertible.hpp>    // bit_convert_source, bit_source, bit_target
#include <xstd/bits/detail/bit_layout.hpp>         // block_byte, bytes_per_block, or_block_byte, value_bytes
#include <boost/dynamic_bitset/dynamic_bitset.hpp> // dynamic_bitset, from_block_range, to_block_range
#include <algorithm>                               // max, min
#include <concepts>                                // same_as
#include <cstddef>                                 // ptrdiff_t, size_t
#include <iterator>                                // output_iterator_tag
#include <ranges>                                  // begin, end, iota, range_value_t, transform
#include <span>                                    // span

// boost::dynamic_bitset's side of bit_convert, by its block ranges: Boost hands none of its storage out.
namespace xstd::bits::detail {

// What to_block_range writes into: source block k lands on its own positions, and what lies past the target drops.
template<class Block, class T>
class block_sink
{
        std::span<T> m_dst;
        std::size_t m_index = 0UZ;

public:
        using iterator_category = std::output_iterator_tag;
        using value_type        = void;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = void;

        [[nodiscard]] constexpr explicit block_sink(std::span<T> dst) noexcept
                : m_dst(dst)
        {}

        [[nodiscard]] constexpr auto operator*() noexcept
                -> block_sink&
        {
                return *this;
        }

        constexpr auto operator++() noexcept
                -> block_sink&
        {
                ++m_index;
                return *this;
        }

        constexpr auto operator=(Block block) noexcept
                -> block_sink&
        {
                if constexpr (std::same_as<Block, T>) {
                        if (m_index < m_dst.size()) {
                                m_dst[m_index] = block;
                        }
                } else {
                        constexpr auto per_block = bytes_per_block<std::span<Block>>;
                        auto const source        = std::span(&block, 1UZ);
                        auto const first         = m_index * per_block;
                        auto const last          = std::ranges::max(first, std::ranges::min(first + per_block, value_bytes(m_dst)));
                        for (auto const j : std::views::iota(first, last)) {
                                or_block_byte(m_dst, j, block_byte(source, j - first));
                        }
                }
                return *this;
        }
};

template<class Block, class AllocatorOrContainer>
struct bit_source<boost::dynamic_bitset<Block, AllocatorOrContainer>>
{
        using bitset_type = boost::dynamic_bitset<Block, AllocatorOrContainer>;

        [[nodiscard]] static constexpr auto width(bitset_type const& from) noexcept
                -> std::size_t
        {
                return from.size();
        }

        [[nodiscard]] static constexpr auto count(bitset_type const& from) noexcept
                -> std::size_t
        {
                return from.count();
        }

        template<class U>
        static constexpr auto copy(bitset_type const& from, std::span<U> dst)
                -> void
        {
                boost::to_block_range(from, block_sink<Block, U>(dst));
        }
};

// Block i at width T of a source's blocks, gathered a byte at a time; a source of T hands its own block over.
template<class T, class Src>
[[nodiscard]] constexpr auto gather_block(Src const& src, std::size_t i) noexcept
        -> T
{
        if constexpr (std::same_as<std::ranges::range_value_t<Src>, T>) {
                return src[i];
        } else {
                constexpr auto per_block = bytes_per_block<std::span<T>>;
                auto block               = T();
                auto const target        = std::span(&block, 1UZ);
                auto const first         = i * per_block;
                for (auto const j : std::views::iota(first, std::ranges::min(first + per_block, value_bytes(src)))) {
                        or_block_byte(target, j - first, block_byte(src, j));
                }
                return block;
        }
}

// A dynamic_bitset made at the source's width, its blocks gathered at its own block width.
template<class Block, class AllocatorOrContainer>
struct bit_target<boost::dynamic_bitset<Block, AllocatorOrContainer>>
{
        using bitset_type = boost::dynamic_bitset<Block, AllocatorOrContainer>;

        // Position i stays i, so the to_string() of the dynamic_bitset made reads reversed, position 0 last.
        template<bit_convert_source From>
                requires requires (From const& from) { bit_source<From>::blocks(from); }
        [[nodiscard]] static constexpr auto convert(From const& from)
                -> bitset_type
        {
                using source        = bit_source<From>;
                auto to             = bitset_type(source::width(from));
                auto const blocks   = source::blocks(from);
                auto const gather   = [&](std::size_t i) -> Block { return gather_block<Block>(blocks, i); };
                auto const gathered = std::views::iota(0UZ, to.num_blocks()) | std::views::transform(gather);
                boost::from_block_range(std::ranges::begin(gathered), std::ranges::end(gathered), to);
                return to;
        }
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_EXT_BOOST_DETAIL_DYNAMIC_BITSET_HPP
