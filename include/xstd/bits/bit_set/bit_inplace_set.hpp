//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP
#define XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP

#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>      // bit_index_mapping
#include <xstd/bits/bit_ranges/from_blocks.hpp>              // from_blocks, from_blocks_t
#include <xstd/bits/bit_set/bit_key_mapping.hpp>             // bit_key_mapping
#include <xstd/bits/bit_set/detail/set_adaptor.hpp>          // admits_width, key_direction, set_adaptor
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp>   // bit_blocks_extent_v
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container
#include <xstd/bits/detail/inplace_blocks.hpp>               // inplace_blocks
#include <xstd/bits/detail/num_blocks.hpp>                   // num_blocks_v, tiling_block
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/rebind.hpp>                       // rebind
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <cstddef>                                           // size_t
#include <functional>                                        // less
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <ranges>                                            // from_range, from_range_t
#include <utility>                                           // forward, move

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: bounded by the type, not by the heap.
template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
        requires bits::detail::tiling_block<Block>
class basic_bit_inplace_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<bits::detail::inplace_blocks<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>
{
        using base_type = bits::detail::set_adaptor<bits::detail::bit_block_container<bits::detail::inplace_blocks<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>;

        // A mapping that names a size closes the universe, and the width must be that size.
        static_assert(bits::detail::set::admits_width<KeyMapping, Key, N>);

public:
        using typename base_type::block_container_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons] less its allocator forms, in [set.overview]'s order; key_compare has no state, so comp is dropped.
        [[nodiscard]] constexpr basic_bit_inplace_set() noexcept
                : basic_bit_inplace_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_inplace_set(key_compare const& /* comp */) noexcept
                : base_type()
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_inplace_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare())
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_inplace_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare())
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr basic_bit_inplace_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare())
                : base_type(il)
        {}

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit a position.
        [[nodiscard]] constexpr basic_bit_inplace_set(from_blocks_t, block_container_type blocks) noexcept
                : base_type(from_blocks, std::move(blocks))
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_inplace_set& x, basic_bit_inplace_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_inplace_set = basic_bit_inplace_set<std::size_t, std::size_t, N>;

// Every bit of the blocks a position, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_inplace_set(from_blocks_t, bits::detail::inplace_blocks<Block, K>) -> basic_bit_inplace_set<std::size_t, Block, bit_blocks_extent_v<Block> * K>;

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct bits::detail::rebind<basic_bit_inplace_set<Key, Block, N, KeyMapping, Compare>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_inplace_set<Key, OtherBlock, N, KeyMapping, Compare>;

        template<std::size_t M>
        using with_width = basic_bit_inplace_set<Key, Block, M, KeyMapping, Compare>;
};

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_BIT_INPLACE_SET_HPP
