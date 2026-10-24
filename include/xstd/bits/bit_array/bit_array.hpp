//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_BIT_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_BIT_ARRAY_HPP

#include <xstd/bits/bit_ranges/from_blocks.hpp>            // from_blocks, from_blocks_t
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp> // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>        // bit_block_container
#include <xstd/bits/detail/num_blocks.hpp>                 // holds_width, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                  // storage, window
#include <xstd/bits/detail/rebind.hpp>                     // rebind
#include <xstd/bits/detail/sequence_adaptor.hpp>           // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>         // unsigned_integer
#include <algorithm>                                       // copy
#include <array>                                           // array
#include <concepts>                                        // constructible_from
#include <cstddef>                                         // size_t
#include <initializer_list>                                // initializer_list

namespace xstd {

// The packed std::array<bool, N>, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
        requires bits::detail::holds_width<Block, N>
class basic_bit_array : public bits::detail::sequence_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_array<Block, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_array<Block, N>>;

public:
        using typename base_type::value_type;

        // std::array is an aggregate and declares none: these are what its initialization does, as constructors.
        [[nodiscard]] basic_bit_array() = default;

        // What is listed leads, the rest stays false.
        constexpr basic_bit_array(std::initializer_list<value_type> il)
                : base_type(il)
        {}

        // Not in [array]: a built-in array of bools, position by position; M spares a zero width from naming bool[0].
        template<std::size_t M>
                requires (M == N)
        [[nodiscard]] constexpr explicit basic_bit_array(value_type const (&a)[M]) noexcept // NOLINT(modernize-avoid-c-arrays): a built-in array is what it converts.
                : base_type()
        {
                std::ranges::copy(a, this->begin());
        }

        // Not in [array]: blocks that are bit storage, read as this sequence's bools.
        template<class Bits>
                requires std::constructible_from<base_type, from_blocks_t, Bits const&>
        [[nodiscard]] constexpr basic_bit_array(from_blocks_t, Bits const& b) noexcept
                : base_type(from_blocks, b)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_array& x, basic_bit_array& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_array = basic_bit_array<std::size_t, N>;

// The width of one block.
template<xstd::unsigned_integer Block>
basic_bit_array(from_blocks_t, Block) -> basic_bit_array<Block, bit_blocks_extent_v<Block>>;

// The width of an array of blocks, zero blocks included, as [span.deduct] takes an array's bound.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_array(from_blocks_t, std::array<Block, K>) -> basic_bit_array<Block, bit_blocks_extent_v<std::array<Block, K>>>;

// A built-in array of blocks, by reference so it keeps its bound: what the std::array of its blocks deduces.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_array(from_blocks_t, Block const (&)[K]) -> basic_bit_array<Block, bit_blocks_extent_v<std::array<Block, K>>>; // NOLINT(modernize-avoid-c-arrays): a built-in array is what it reads.

template<class Block, std::size_t N>
struct bits::detail::rebind<basic_bit_array<Block, N>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_array<OtherBlock, N>;

        template<std::size_t M>
        using with_width = basic_bit_array<Block, M>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_ARRAY_BIT_ARRAY_HPP
