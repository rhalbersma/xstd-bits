//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage_extent_v
#include <xstd/bits/detail/bitset_adaptor.hpp>           // bitset_adaptor
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, num_blocks_v
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage, from_bit_storage_t
#include <xstd/ints/concepts/unsigned_integer.hpp>       // unsigned_integer
#include <xstd/ints/limits.hpp>                          // numeric_limits
#include <xstd/ints/memory.hpp>                          // align_up
#include <array>                                         // array
#include <concepts>                                      // constructible_from
#include <cstddef>                                       // size_t
#include <functional>                                    // hash
#include <limits>                                        // numeric_limits
#include <string>                                        // basic_string
#include <string_view>                                   // basic_string_view

namespace xstd {

// [template.bitset] over a packed array of Block: what std::bitset<N> is, with the word type in the open.
template<xstd::unsigned_integer Block, std::size_t N>
class basic_bitset : public bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bitset<Block, N>>
{
        using base_type = bits::detail::bitset_adaptor<bits::detail::contiguous_bit_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, basic_bitset<Block, N>>;

public:
        // [bitset.cons], in its order.
        [[nodiscard]] basic_bitset() noexcept = default;

        [[nodiscard]] constexpr explicit(false) basic_bitset(unsigned long long val) noexcept // NOLINT(misc-explicit-constructor)
                : base_type(val)
        {}

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit basic_bitset(
                std::basic_string<charT, traits, Allocator> const& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit basic_bitset(
                std::basic_string_view<charT, traits> str,
                std::basic_string_view<charT, traits>::size_type pos = 0,
                std::basic_string_view<charT, traits>::size_type n = std::basic_string_view<charT, traits>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, pos, n, zero, one)
        {}

        template<class charT>
                requires std::constructible_from<base_type, charT const*>
        [[nodiscard]] constexpr explicit basic_bitset(
                charT const* str,
                std::size_t n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : base_type(str, n, zero, one)
        {}

        [[nodiscard]] basic_bitset(basic_bitset const& x) = default;
        [[nodiscard]] basic_bitset(basic_bitset&& x) = default;

        ~basic_bitset() = default;

        auto operator=(basic_bitset const& x) -> basic_bitset& = default;
        auto operator=(basic_bitset&& x) noexcept -> basic_bitset& = default;

        // Not in [bitset.cons]: words that are bit storage, integers wider than the ullong door included.
        template<class B>
                requires std::constructible_from<base_type, from_bit_storage_t, B const&>
        [[nodiscard]] constexpr basic_bitset(from_bit_storage_t, B const& b) noexcept
                : base_type(from_bit_storage, b)
        {}

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bitset& x, basic_bitset& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

// The width of one word or of an array of them; K = 1 keeps MSVC 17 from dropping the one-word guide.
template<xstd::unsigned_integer Block, std::size_t K = 1>
basic_bitset(from_bit_storage_t, Block) -> basic_bitset<Block, bit_storage_extent_v<Block> * K>;

// No guide from zero blocks: an empty array names no width worth deducing.
template<xstd::unsigned_integer Block, std::size_t K>
        requires (K != 0)
basic_bitset(from_bit_storage_t, std::array<Block, K>) -> basic_bitset<Block, bit_storage_extent_v<std::array<Block, K>>>;

// std::bitset's integer constructor at the width of the integer's type, where that constructor reads every digit.
template<xstd::unsigned_integer Block, std::size_t K = 1>
        requires (xstd::numeric_limits<Block>::digits <= std::numeric_limits<unsigned long long>::digits)
basic_bitset(Block) -> basic_bitset<std::size_t, bit_storage_extent_v<Block> * K>;

// The width rounded up to whole blocks: no unused tail, so every block is the value.
namespace aligned {

template<xstd::unsigned_integer Block, std::size_t N>
using basic_bitset = xstd::basic_bitset<Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits))>;

template<std::size_t N>
using bitset = basic_bitset<std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

template<class Block, std::size_t N>
struct hash<xstd::basic_bitset<Block, N>> : hash<typename xstd::basic_bitset<Block, N>::adaptor_type>
{};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

#endif // XSTD_BITS_BITSET_HPP
