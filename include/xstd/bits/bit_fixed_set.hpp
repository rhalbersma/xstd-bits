//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FIXED_SET_HPP
#define XSTD_BITS_BIT_FIXED_SET_HPP

#include <xstd/bits/bit_concepts/bit_block.hpp>               // bit_block
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>       // bit_index_mapping
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp>        // bit_mask_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_key_mapping.hpp>                      // bit_key_mapping
#include <xstd/bits/bit_type_traits/bit_blocks_extent.hpp>    // bit_blocks_extent_v
#include <xstd/bits/bit_type_traits/bit_least.hpp>            // least_block_t
#include <xstd/bits/detail/bit_block_container.hpp>           // bit_block_container, num_blocks_v
#include <xstd/bits/detail/bit_layout.hpp>                    // byte_count, bytes_bits
#include <xstd/bits/detail/ownership.hpp>                     // storage, storage_access
#include <xstd/bits/detail/rebind.hpp>                        // rebind
#include <xstd/bits/detail/set_adaptor.hpp>                   // admits_width, disjoint, intersects, key_direction, set_adaptor
#include <xstd/bits/detail/shift.hpp>                         // shr
#include <xstd/bits/from_blocks.hpp>                          // from_blocks, from_blocks_t
#include <xstd/ints/concepts/unsigned_integer.hpp>            // unsigned_integer
#include <xstd/ints/limits.hpp>                               // numeric_limits
#include <xstd/misc/concepts/container_compatible_range.hpp>  // container_compatible_range
#include <boost/container_hash/is_range.hpp>                  // is_range
#include <boost/container_hash/is_tuple_like.hpp>             // is_tuple_like
#include <array>                                              // array
#include <cassert>                                            // assert
#include <concepts>                                           // constructible_from, same_as
#include <cstddef>                                            // size_t
#include <functional>                                         // hash, less
#include <initializer_list>                                   // initializer_list
#include <iterator>                                           // input_iterator
#include <ranges>                                             // from_range, from_range_t
#include <type_traits>                                        // conditional_t, false_type, is_enum_v
#include <utility>                                            // forward

namespace xstd {

// The fixed-width set: the basic name leaves the key and the block open, the short one makes both std::size_t.
template<class Key, xstd::unsigned_integer Block, std::size_t N, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, bits::detail::set::key_direction<Key> Compare = std::less<Key>>
class basic_bit_fixed_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>
{
        using base_type = bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>, Key, KeyMapping, Compare>;

        // A mapping that names a size closes the universe, and the width must be that size.
        static_assert(bits::detail::set::admits_width<KeyMapping, Key, N>);

        // Each value of the key a mask of its one-bit keys, which converts with the set both ways.
        static constexpr bool is_mask = bit_mask_mapping<KeyMapping, Key>;

        // No caller can make one, so the mask constructor taking it is unreachable when Key is no mask.
        class not_a_mask
        {
                not_a_mask() = default;
        };

        // A type rather than a constraint, since MSVC drops an inherited constructor's requires-clause candidate.
        using mask_type = std::conditional_t<is_mask, Key, not_a_mask>;

public:
        using typename base_type::key_compare;
        using typename base_type::key_type;
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

        // A mask's list is the union of its values, so {m} is the conversion from m and {a, b} two one-bit values.
        [[nodiscard]] constexpr basic_bit_fixed_set(std::initializer_list<value_type> il, key_compare const& /* comp */ = key_compare()) noexcept(is_mask)
        {
                if constexpr (is_mask) {
                        for (auto const& mask : il) {
                                *this |= mask;
                        }
                } else {
                        this->insert(il.begin(), il.end());
                }
        }

        // Not in [set.cons]: blocks that are bit storage, read as this set's positions.
        template<class Bits>
                requires std::constructible_from<base_type, from_blocks_t, Bits const&>
        [[nodiscard]] constexpr basic_bit_fixed_set(from_blocks_t, Bits const& b) noexcept
                : base_type(from_blocks, b)
        {}

        // Not in [set.cons]: any value of a mask, as the enumeration it replaces takes it; no bit at or above N.
        [[nodiscard]] constexpr explicit(false) basic_bit_fixed_set(mask_type const& mask) noexcept // NOLINT(misc-explicit-constructor)
        {
                assert(fits(mask));
                bits::detail::storage_access::bits(*this).assign_bits(KeyMapping::to_block(mask));
        }

        using base_type::operator=;

        // The mask with this set's keys, every bit of it at or above N clear.
        [[nodiscard]] constexpr explicit(false) operator key_type() const noexcept // NOLINT(misc-explicit-constructor)
                requires is_mask
        {
                using mask_block = KeyMapping::block_type;
                return KeyMapping::from_block(bits::detail::bytes_bits<mask_block, N>(bits::detail::storage_access::bits(*this).template to_bytes<bits::detail::byte_count<N>>()));
        }

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(basic_bit_fixed_set& x, basic_bit_fixed_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }

        // The base's queries again, here so a mask converts: on either side of the friends, as the members' argument.
        [[nodiscard]] friend constexpr auto intersects(basic_bit_fixed_set const& x, basic_bit_fixed_set const& y) noexcept
                -> bool
                requires is_mask
        {
                return intersects(static_cast<base_type const&>(x), static_cast<base_type const&>(y));
        }

        [[nodiscard]] friend constexpr auto disjoint(basic_bit_fixed_set const& x, basic_bit_fixed_set const& y) noexcept
                -> bool
                requires is_mask
        {
                return disjoint(static_cast<base_type const&>(x), static_cast<base_type const&>(y));
        }

        // A member hides the base's of its name, so these bring the base's set forms back beside the mask ones.
        using base_type::is_proper_subset_of;
        using base_type::is_proper_superset_of;
        using base_type::is_subset_of;
        using base_type::is_superset_of;

        [[nodiscard]] constexpr auto is_subset_of(basic_bit_fixed_set const& other) const noexcept
                -> bool
                requires is_mask
        {
                return base_type::is_subset_of(other);
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(basic_bit_fixed_set const& other) const noexcept
                -> bool
                requires is_mask
        {
                return base_type::is_proper_subset_of(other);
        }

        [[nodiscard]] constexpr auto is_superset_of(basic_bit_fixed_set const& other) const noexcept
                -> bool
                requires is_mask
        {
                return base_type::is_superset_of(other);
        }

        [[nodiscard]] constexpr auto is_proper_superset_of(basic_bit_fixed_set const& other) const noexcept
                -> bool
                requires is_mask
        {
                return base_type::is_proper_superset_of(other);
        }

        // Total, as the binary & and - are: the value's bits at or above N meet nothing in this set.
        friend constexpr auto operator&=(basic_bit_fixed_set& lhs, key_type const& rhs) noexcept
                -> basic_bit_fixed_set&
                requires is_mask
        {
                return lhs &= low_bits_of(rhs);
        }

        friend constexpr auto operator-=(basic_bit_fixed_set& lhs, key_type const& rhs) noexcept
                -> basic_bit_fixed_set&
                requires is_mask
        {
                return lhs -= low_bits_of(rhs);
        }

        // These two would bring the value's bits above N into the set, which they must not have.
        friend constexpr auto operator|=(basic_bit_fixed_set& lhs, key_type const& rhs) noexcept
                -> basic_bit_fixed_set&
                requires is_mask
        {
                return lhs |= basic_bit_fixed_set(rhs);
        }

        friend constexpr auto operator^=(basic_bit_fixed_set& lhs, key_type const& rhs) noexcept
                -> basic_bit_fixed_set&
                requires is_mask
        {
                return lhs ^= basic_bit_fixed_set(rhs);
        }

        // Templates over the mask's exact type, so that a set converting to it is never deduced as one.
        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator==(basic_bit_fixed_set const& lhs, M const& rhs) noexcept
                -> bool
                requires is_mask
        {
                return fits(rhs) and lhs == low_bits_of(rhs);
        }

        // The rest in both orders, exact where the base's would take the value as one key; no bit at or above N.
        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator|(basic_bit_fixed_set lhs, M const& rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return lhs |= rhs;
        }

        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator|(M const& lhs, basic_bit_fixed_set rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return rhs |= lhs;
        }

        // Total: truncating the value to N bits is exact, since the set has no bit above N for it to keep.
        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator&(basic_bit_fixed_set lhs, M const& rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return lhs &= rhs;
        }

        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator&(M const& lhs, basic_bit_fixed_set rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return rhs &= lhs;
        }

        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator^(basic_bit_fixed_set lhs, M const& rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return lhs ^= rhs;
        }

        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator^(M const& lhs, basic_bit_fixed_set rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return rhs ^= lhs;
        }

        // Total, as & is; the mirror is not, the value's bits above N being what it would keep.
        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator-(basic_bit_fixed_set lhs, M const& rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return lhs -= rhs;
        }

        template<std::same_as<key_type> M>
        [[nodiscard]] friend constexpr auto operator-(M const& lhs, basic_bit_fixed_set const& rhs) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                auto nrv = basic_bit_fixed_set(lhs);
                nrv -= rhs;
                return nrv;
        }

private:
        // Where a write brings a mask's bits into the set, the mask must have none at or above N.
        [[nodiscard]] static constexpr auto fits(key_type const& mask) noexcept
                -> bool
                requires is_mask
        {
                using mask_block = KeyMapping::block_type;
                if constexpr (N < static_cast<std::size_t>(xstd::numeric_limits<mask_block>::digits)) {
                        return bits::detail::shr(KeyMapping::to_block(mask), N) == mask_block{};
                } else {
                        return true;
                }
        }

        // The mask's bits below N, all of it that can meet this set.
        [[nodiscard]] static constexpr auto low_bits_of(key_type const& mask) noexcept
                -> basic_bit_fixed_set
                requires is_mask
        {
                return basic_bit_fixed_set(from_blocks, KeyMapping::to_block(mask));
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

// A built-in array of blocks, by reference so it keeps its bound: what the std::array of its blocks deduces.
template<xstd::bit_block Block, std::size_t K>
basic_bit_fixed_set(from_blocks_t, Block const (&)[K]) -> basic_bit_fixed_set<std::size_t, Block, bit_blocks_extent_v<std::array<Block, K>>>; // NOLINT(modernize-avoid-c-arrays): a built-in array is what it reads.

// A list of enumerators whose default mapping closes the universe: the type bit_enum_set<Enum> names.
template<class Enum>
        requires std::is_enum_v<Enum> and sized_bit_index_mapping<bit_key_mapping<Enum>, Enum>
basic_bit_fixed_set(std::initializer_list<Enum>) -> basic_bit_fixed_set<Enum, least_block_t<bit_key_mapping<Enum>::size>, bit_key_mapping<Enum>::size, bit_key_mapping<Enum>>;

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct bits::detail::rebind<basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>>
{
        using block_type                   = Block;
        static constexpr std::size_t width = N;

        template<class OtherBlock>
        using with_block = basic_bit_fixed_set<Key, OtherBlock, N, KeyMapping, Compare>;

        template<std::size_t M>
        using with_width = basic_bit_fixed_set<Key, Block, M, KeyMapping, Compare>;
};

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_range<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct is_tuple_like<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Key, class Block, std::size_t N, class KeyMapping, class Compare>
struct hash<xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>> : hash<typename xstd::basic_bit_fixed_set<Key, Block, N, KeyMapping, Compare>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_FIXED_SET_HPP
