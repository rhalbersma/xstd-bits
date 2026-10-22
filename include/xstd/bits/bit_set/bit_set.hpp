//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_SET_BIT_SET_HPP
#define XSTD_BITS_BIT_SET_BIT_SET_HPP

#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>      // bit_index_mapping
#include <xstd/bits/bit_concepts/bit_storage.hpp>            // bit_storage
#include <xstd/bits/bit_ranges/from_blocks.hpp>              // from_blocks, from_blocks_t
#include <xstd/bits/bit_set/bit_key_mapping.hpp>             // bit_key_mapping
#include <xstd/bits/bit_set/detail/set_adaptor.hpp>          // admits_width, key_direction, set_adaptor
#include <xstd/bits/detail/allocator_base_type.hpp>          // allocator_aware, allocator_param_t
#include <xstd/bits/detail/bit_block_container.hpp>          // set_bits_t
#include <xstd/bits/detail/ownership.hpp>                    // storage
#include <xstd/bits/detail/rebind.hpp>                       // rebind
#include <xstd/bits/detail/storage_rebind.hpp>               // fits_storage, storage_owner, storage_rebind, storage_width_v
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <xstd/misc/concepts/simple_allocator.hpp>           // simple_allocator
#include <cstddef>                                           // size_t
#include <functional>                                        // less
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <memory>                                            // allocator
#include <ranges>                                            // from_range, from_range_t, input_range
#include <span>                                              // dynamic_extent
#include <type_traits>                                       // is_nothrow_move_constructible_v
#include <utility>                                           // forward, move
#include <vector>                                            // vector

namespace xstd {

// The set reading over a storage of blocks, at most N positions of them: the flagship, and the one name without a qualifier.
template<class Key, bit_storage Blocks = std::vector<std::size_t>, std::size_t N = std::dynamic_extent, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
        requires bits::detail::fits_storage<Blocks, N>
class basic_bit_set : public bits::detail::set_adaptor<bits::detail::set_bits_t<Blocks, bits::detail::storage_width_v<Blocks, N>>, bits::detail::storage::owned, basic_bit_set<Key, Blocks, N, KeyMapping, Compare>, Key, KeyMapping, Compare>
{
        using base_type = bits::detail::set_adaptor<bits::detail::set_bits_t<Blocks, bits::detail::storage_width_v<Blocks, N>>, bits::detail::storage::owned, basic_bit_set<Key, Blocks, N, KeyMapping, Compare>, Key, KeyMapping, Compare>;

        static constexpr std::size_t width = bits::detail::storage_width_v<Blocks, N>;

        // A mapping that names a size closes the universe, and a width the type fixes must be that size.
        static_assert(width == std::dynamic_extent or bits::detail::set::admits_width<KeyMapping, Key, width>);

        // An allocator argument where the storage has an allocator, and no overload taking one where it has none.
        static constexpr bool is_allocator_aware = bits::detail::allocator_aware<Blocks>;

        using allocator_param = bits::detail::allocator_param_t<Blocks>;

public:
        using typename base_type::block_container_type;
        using typename base_type::key_compare;
        using typename base_type::value_type;

        // [set.cons], in [set.overview]'s order, each beside its allocator-extended form; key_compare has no state.
        [[nodiscard]] basic_bit_set() = default;

        [[nodiscard]] constexpr explicit basic_bit_set(key_compare const& /* comp */) noexcept(std::is_nothrow_default_constructible_v<base_type>)
                : base_type()
        {}

        [[nodiscard]] constexpr basic_bit_set(key_compare const& /* comp */, allocator_param const& a) noexcept(std::is_nothrow_constructible_v<base_type, allocator_param const&>)
                requires is_allocator_aware
                : base_type(a)
        {}

        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr basic_bit_set(InputIterator first, InputIterator last, key_compare const& /* comp */ = key_compare())
                : base_type(first, last)
        {}

        template<std::input_iterator InputIterator>
                requires is_allocator_aware
        [[nodiscard]] constexpr basic_bit_set(InputIterator first, InputIterator last, key_compare const& /* comp */, allocator_param const& a)
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, key_compare const& /* comp */ = key_compare())
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        template<xstd::container_compatible_range<value_type> R>
                requires is_allocator_aware
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, key_compare const& /* comp */, allocator_param const& a)
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] constexpr explicit basic_bit_set(allocator_param const& a) noexcept(std::is_nothrow_constructible_v<base_type, allocator_param const&>)
                requires is_allocator_aware
                : base_type(a)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set const& x, allocator_param const& a)
                requires is_allocator_aware
                : base_type(x, a)
        {}

        [[nodiscard]] constexpr basic_bit_set(basic_bit_set&& x, allocator_param const& a)
                requires is_allocator_aware
                : base_type(std::move(x), a)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare())
                : base_type(il)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, key_compare const& /* comp */, allocator_param const& a)
                requires is_allocator_aware
                : base_type(il, a)
        {}

        template<std::input_iterator InputIterator>
                requires is_allocator_aware
        [[nodiscard]] constexpr basic_bit_set(InputIterator first, InputIterator last, allocator_param const& a)
                : base_type(first, last, a)
        {}

        template<xstd::container_compatible_range<value_type> R>
                requires is_allocator_aware
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg, allocator_param const& a)
                : base_type(std::from_range, std::forward<R>(rg), a)
        {}

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il, allocator_param const& a)
                requires is_allocator_aware
                : base_type(il, a)
        {}

        // Not in [set.cons]: flat_set's container constructor under the bit-storage tag, every bit a position.
        [[nodiscard]] constexpr basic_bit_set(from_blocks_t, block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<block_container_type>)
                : base_type(from_blocks, std::move(blocks))
        {}

        [[nodiscard]] constexpr basic_bit_set(from_blocks_t, block_container_type blocks, allocator_param const& a)
                requires is_allocator_aware
                : base_type(from_blocks, std::move(blocks), a)
        {}

        using base_type::operator=;

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_set& x, basic_bit_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }
};

using bit_set = basic_bit_set<std::size_t>;

// [set.overview]'s guides, in its order: the blocks from the allocator, std::size_t by default, the key being std::size_t.
template<std::input_iterator InputIterator, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(InputIterator, InputIterator, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>, std::dynamic_extent, bit_key_mapping<std::size_t>, Compare>;

template<std::ranges::input_range R, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(std::from_range_t, R&&, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>, std::dynamic_extent, bit_key_mapping<std::size_t>, Compare>;

template<class Key, class Compare = std::less<std::size_t>, class Allocator = std::allocator<std::size_t>>
        requires (not xstd::simple_allocator<Compare>) and xstd::simple_allocator<Allocator>
basic_bit_set(std::initializer_list<Key>, Compare = Compare(), Allocator = Allocator()) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>, std::dynamic_extent, bit_key_mapping<std::size_t>, Compare>;

template<std::input_iterator InputIterator, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(InputIterator, InputIterator, Allocator) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>>;

template<std::ranges::input_range R, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(std::from_range_t, R&&, Allocator) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>>;

template<class Key, class Allocator>
        requires xstd::simple_allocator<Allocator>
basic_bit_set(std::initializer_list<Key>, Allocator) -> basic_bit_set<std::size_t, std::vector<typename Allocator::value_type, Allocator>>;

// The blocks adopted are the storage, every bit of them a position: a constant capacity becomes the cap.
template<bit_storage Blocks>
basic_bit_set(from_blocks_t, Blocks) -> basic_bit_set<std::size_t, Blocks, bits::detail::storage_width_v<Blocks, std::dynamic_extent>>;

template<bit_storage Blocks, class Allocator>
basic_bit_set(from_blocks_t, Blocks, Allocator) -> basic_bit_set<std::size_t, Blocks, bits::detail::storage_width_v<Blocks, std::dynamic_extent>>;

// Another block or width goes through the storage's bit_storage_rebind, the key, mapping and comparator kept.
template<class Key, class Blocks, std::size_t N, class KeyMapping, class Compare>
struct bits::detail::storage_owner<basic_bit_set<Key, Blocks, N, KeyMapping, Compare>>
{
        template<class OtherBlocks, std::size_t M>
        using type = basic_bit_set<Key, OtherBlocks, M, KeyMapping, Compare>;
};

template<class Key, class Blocks, std::size_t N, class KeyMapping, class Compare>
struct bits::detail::rebind<basic_bit_set<Key, Blocks, N, KeyMapping, Compare>> : bits::detail::storage_rebind<basic_bit_set<Key, Blocks, N, KeyMapping, Compare>, Blocks, N>
{};

} // namespace xstd

#endif // XSTD_BITS_BIT_SET_BIT_SET_HPP
