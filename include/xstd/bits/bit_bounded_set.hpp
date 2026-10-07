//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_BOUNDED_SET_HPP
#define XSTD_BITS_BIT_BOUNDED_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                          // bit_blocks_extent_v
#include <xstd/bits/bit_index_mapping.hpp>                   // bit_index_mapping
#include <xstd/bits/bit_key_mapping.hpp>                     // bit_key_mapping
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container, num_blocks_v
#include <xstd/bits/detail/bounded_blocks.hpp>               // bounded_blocks
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/set_adaptor.hpp>                  // admits_width, key_direction, set_adaptor
#include <xstd/bits/from_blocks.hpp>                         // from_blocks, from_blocks_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/ints/memory.hpp>                              // align_up
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <cstddef>                                           // size_t
#include <functional>                                        // hash, less
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <limits>                                            // numeric_limits
#include <ranges>                                            // from_range, from_range_t
#include <type_traits>                                       // false_type
#include <utility>                                           // forward, move

namespace xstd {

// The set reading over a run-time width under a compile-time capacity: bounded by the type, not by the heap.
template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
class basic_bit_bounded_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<bits::detail::bounded_blocks<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>
{
        using base_type = bits::detail::set_adaptor<bits::detail::bit_block_container<bits::detail::bounded_blocks<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>;

        // A mapping that names a size closes the universe, and the width must be that size.
        static_assert(bits::detail::set::admits_width<KeyMapping, Key, N>);

public:
        using typename base_type::block_container_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons] less its allocator forms, in [set.overview]'s order; key_compare has no state, so comp is dropped.
        [[nodiscard]] constexpr basic_bit_bounded_set() noexcept
                : basic_bit_bounded_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_bounded_set(key_compare const& /* comp */) noexcept
                : base_type()
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_bounded_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare())
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_bounded_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare())
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr basic_bit_bounded_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare())
                : base_type(il)
        {}

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit a position.
        [[nodiscard]] constexpr basic_bit_bounded_set(from_blocks_t, block_container_type blocks) noexcept
                : base_type(from_blocks, std::move(blocks))
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_bounded_set& x, basic_bit_bounded_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_bounded_set = basic_bit_bounded_set<std::size_t, std::size_t, N>;

// Every bit of the blocks a position, so the capacity is theirs, rounded to whole blocks.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_bounded_set(from_blocks_t, bits::detail::bounded_blocks<Block, K>) -> basic_bit_bounded_set<std::size_t, Block, bit_blocks_extent_v<Block> * K>;

namespace aligned {

template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, class Compare = std::less<Key>>
using basic_bit_bounded_set = xstd::basic_bit_bounded_set<Key, Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), KeyMapping, Compare>;

template<std::size_t N>
using bit_bounded_set = basic_bit_bounded_set<std::size_t, std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_range<xstd::basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_tuple_like<xstd::basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct hash<xstd::basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>> : hash<typename xstd::basic_bit_bounded_set<Key, Block, N, KeyMapping, Compare>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_BOUNDED_SET_HPP
