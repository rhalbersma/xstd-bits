//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP
#define XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP

#include <xstd/bits/bit_ranges/from_blocks.hpp>              // from_blocks, from_blocks_t
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp>   // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container
#include <xstd/bits/detail/inplace_blocks.hpp>               // inplace_blocks, inplace_blocks_for
#include <xstd/bits/detail/num_blocks.hpp>                   // num_blocks_v, tiling_block
#include <xstd/bits/detail/ownership.hpp>                    // storage, window
#include <xstd/bits/detail/rebind.hpp>                       // rebind
#include <xstd/bits/detail/sequence_adaptor.hpp>             // sequence_adaptor
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <cstddef>                                           // size_t
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <ranges>                                            // from_range, from_range_t
#include <utility>                                           // forward, move

namespace xstd {

// The packed std::inplace_vector<bool, N> that P0843 declined to write, named after the container it packs.
template<xstd::unsigned_integer Block, std::size_t N>
        requires bits::detail::tiling_block<Block>
class basic_bit_inplace_vector : public bits::detail::sequence_adaptor<bits::detail::bit_block_container<bits::detail::inplace_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_inplace_vector<Block, N>>
{
        using base_type = bits::detail::sequence_adaptor<bits::detail::bit_block_container<bits::detail::inplace_blocks_for<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, bits::detail::window::all, basic_bit_inplace_vector<Block, N>>;

public:
        using typename base_type::block_container_type;
        using typename base_type::size_type;

        // [inplace.vector.cons], in [inplace.vector.overview]'s order.
        [[nodiscard]] basic_bit_inplace_vector() = default;

        [[nodiscard]] constexpr explicit basic_bit_inplace_vector(size_type n)
                : base_type(n)
        {}

        [[nodiscard]] constexpr basic_bit_inplace_vector(size_type n, bool const& value)
                : base_type(n, value)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_inplace_vector(InputIterator first, InputIterator last)
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<bool> R>
        [[nodiscard]] constexpr basic_bit_inplace_vector(std::from_range_t, R&& rg)
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr basic_bit_inplace_vector(std::initializer_list<bool> il)
                : base_type(il)
        {}

        // Not in [inplace.vector.cons]: flat_set's container constructor under the bit-storage tag.
        [[nodiscard]] constexpr basic_bit_inplace_vector(from_blocks_t, block_container_type blocks) noexcept
                : base_type(from_blocks, std::move(blocks))
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_inplace_vector& x, basic_bit_inplace_vector& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_inplace_vector = basic_bit_inplace_vector<std::size_t, N>;

// Every bit of the blocks an element, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_inplace_vector(from_blocks_t, bits::detail::inplace_blocks<Block, K>) -> basic_bit_inplace_vector<Block, bit_blocks_extent_v<Block> * K>;

template<class Block, std::size_t N>
struct bits::detail::rebind<basic_bit_inplace_vector<Block, N>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_inplace_vector<OtherBlock, N>;

        template<std::size_t M>
        using with_width = basic_bit_inplace_vector<Block, M>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_INPLACE_VECTOR_BIT_INPLACE_VECTOR_HPP
