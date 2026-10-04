//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FIXED_SET_HPP
#define XSTD_BITS_BIT_FIXED_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                          // bit_blocks_extent_v
#include <xstd/bits/bit_key_traits.hpp>                      // bit_key_traits
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container, num_blocks_v
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/set_adaptor.hpp>                  // admits_width, key_direction, set_adaptor
#include <xstd/bits/from_blocks.hpp>                         // from_blocks, from_blocks_t
#include <xstd/ints/concepts/unsigned_integer.hpp>           // unsigned_integer
#include <xstd/ints/memory.hpp>                              // align_up
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <array>                                             // array
#include <concepts>                                          // constructible_from
#include <cstddef>                                           // size_t
#include <functional>                                        // hash, less
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <limits>                                            // numeric_limits
#include <ranges>                                            // from_range, from_range_t
#include <type_traits>                                       // false_type
#include <utility>                                           // forward

namespace xstd {

// The fixed-width set: the basic name leaves the key and the block open, the short one makes both std::size_t.
template<class Key, xstd::unsigned_integer Block, std::size_t N, class KeyTraits = bit_key_traits<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
class basic_bit_fixed_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>, Key, KeyTraits, Compare>
{
        using base_type = bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>, Key, KeyTraits, Compare>;

        // A traits type that names a size closes the universe, and the width must be that size.
        static_assert(bits::detail::set::admits_width<KeyTraits, N>);

public:
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons] less its allocator forms, in [set.overview]'s order; key_compare has no state, so comp is dropped.
        [[nodiscard]] constexpr basic_bit_fixed_set() noexcept
                : basic_bit_fixed_set(key_compare())
        {}

        [[nodiscard]] constexpr explicit basic_bit_fixed_set(key_compare const& /* comp */) noexcept
                : base_type()
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_fixed_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare())
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_fixed_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare())
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr basic_bit_fixed_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare())
                : base_type(il)
        {}

        // Not in [set.cons]: blocks that are bit storage, read as this set's positions.
        template<class B>
                requires std::constructible_from<base_type, from_blocks_t, B const&>
        [[nodiscard]] constexpr basic_bit_fixed_set(from_blocks_t, B const& b) noexcept
                : base_type(from_blocks, b)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_fixed_set& x, basic_bit_fixed_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

template<std::size_t N>
using bit_fixed_set = basic_bit_fixed_set<std::size_t, std::size_t, N>;

// The width of one block.
template<xstd::unsigned_integer Block>
basic_bit_fixed_set(from_blocks_t, Block) -> basic_bit_fixed_set<std::size_t, Block, bit_blocks_extent_v<Block>>;

// The width of an array of blocks, zero blocks included, as [span.deduct] takes an array's bound.
template<xstd::unsigned_integer Block, std::size_t K>
basic_bit_fixed_set(from_blocks_t, std::array<Block, K>) -> basic_bit_fixed_set<std::size_t, Block, bit_blocks_extent_v<std::array<Block, K>>>;

namespace aligned {

template<class Key, xstd::unsigned_integer Block, std::size_t N, class KeyTraits = bit_key_traits<Key>, class Compare = std::less<Key>>
using basic_bit_fixed_set = xstd::basic_bit_fixed_set<Key, Block, xstd::align_up(N, static_cast<std::size_t>(std::numeric_limits<Block>::digits)), KeyTraits, Compare>;

template<std::size_t N>
using bit_fixed_set = basic_bit_fixed_set<std::size_t, std::size_t, N>;

} // namespace aligned

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Key, class Block, std::size_t N, class KeyTraits, class Compare>
struct is_range<xstd::basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>> : std::false_type
{};

template<class Key, class Block, std::size_t N, class KeyTraits, class Compare>
struct is_tuple_like<xstd::basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.
namespace std {

template<class Key, class Block, std::size_t N, class KeyTraits, class Compare>
struct hash<xstd::basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>> : hash<typename xstd::basic_bit_fixed_set<Key, Block, N, KeyTraits, Compare>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_FIXED_SET_HPP
