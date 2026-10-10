//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SET_ADAPTOR_HPP
#define XSTD_BITS_DETAIL_SET_ADAPTOR_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>              // bit_blocks
#include <xstd/bits/bit_concepts/bit_index_mapping.hpp>       // bit_index_mapping
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp>        // bit_mask_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_hasher.hpp>                           // bit_hasher
#include <xstd/bits/bit_key_mapping.hpp>                      // bit_key_mapping
#include <xstd/bits/detail/adapted_bits.hpp>                  // adapted_bits
#include <xstd/bits/detail/allocator_base_type.hpp>           // allocator_aware, allocator_base_type, allocator_param_t
#include <xstd/bits/detail/bit_block_container.hpp>           // bit_block_container, bit_block_container_type
#include <xstd/bits/detail/borrowed_bits.hpp>                 // borrow_bits, borrowable_block, borrowable_blocks, borrowed_bits_t
#include <xstd/bits/detail/comparisons.hpp>                   // numeric_three_way, set_equal, three_way
#include <xstd/bits/detail/functor.hpp>                       // decay_copy
#include <xstd/bits/detail/hash.hpp>                          // hash_append_keys
#include <xstd/bits/detail/intrin.hpp>                        // countl_zero, countr_zero
#include <xstd/bits/detail/is_key.hpp>                        // is_key
#include <xstd/bits/detail/owner_members.hpp>                 // owner_members
#include <xstd/bits/detail/ownership.hpp>                     // owned_bits_t, owned_storage, owner_of, owns, reads, set_reading_tag, storage, storage_access
#include <xstd/bits/detail/shift.hpp>                         // shl, shr
#include <xstd/bits/detail/storage_ptr.hpp>                   // storage_ptr_t, storage_ref_t
#include <xstd/bits/from_blocks.hpp>                          // from_blocks_t
#include <xstd/misc/concepts/specialization_of.hpp>           // specialization_of
#include <xstd/misc/type_traits/empty_base_type.hpp>          // empty_base_type
#include <boost/container_hash/is_range.hpp>                  // is_range
#include <boost/hash2/hash_append_fwd.hpp>                    // hash_append_tag
#include <boost/hash2/xxhash.hpp>                             // xxhash_64
#include <algorithm>                                          // all_of, find_if, max, min, partition_point
#include <cassert>                                            // assert
#include <compare>                                            // strong_ordering
#include <concepts>                                           // constructible_from, convertible_to, equality_comparable, invocable, same_as, totally_ordered
#include <cstddef>                                            // ptrdiff_t, size_t
#include <format>                                             // format, formatter
#include <functional>                                         // greater, hash, less
#include <initializer_list>                                   // initializer_list
#include <iterator>                                           // bidirectional_iterator_tag, input_iterator, iter_reference_t, make_reverse_iterator, reverse_iterator, sentinel_for
#include <ranges>                                             // begin, enable_borrowed_range, enable_view, end, input_range, iota, iota_view, range_reference_t, from_range_t, subrange, swap, transform
#include <source_location>                                    // source_location
#include <span>                                               // dynamic_extent
#include <stdexcept>                                          // out_of_range
#include <type_traits>                                        // conditional_t, false_type, integral_constant, is_enum_v, is_invocable_r_v, is_nothrow_constructible_v, is_nothrow_default_constructible_v, is_nothrow_move_constructible_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                                            // declval, forward, in_place, move, pair

// The set reading, [set] over a bit_block_container, owning it or referring to it.
namespace xstd::bits::detail {

namespace set {

// One tier each: sharing a body puts the whole over readability-function-cognitive-complexity.

// Blocks, lowest position first: load once per block, then tzcnt for the position and blsr to drop it.
template<class Key, class KeyMapping, class Bits, class F>
constexpr auto walk_blocks_ascending(Bits const& c, F& f)
        -> void
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;

        for (auto const index : std::views::iota(0UZ, c.num_blocks())) {
                auto block = c[index];
                while (block != block_type{}) {
                        auto const offset = static_cast<std::size_t>(countr_zero(block));
                        // A functor returning void has no exit to take, so its walk is compiled without one.
                        if constexpr (std::is_invocable_r_v<bool, F&, Key>) {
                                if (not f(decay_copy<Key>(KeyMapping::from_index((digits * index) + offset)))) {
                                        return;
                                }
                        } else {
                                f(decay_copy<Key>(KeyMapping::from_index((digits * index) + offset)));
                        }
                        block = static_cast<block_type>(block & static_cast<block_type>(block - block_type{1}));
                }
        }
}

// The mirror. w & (w - 1) has no descending twin, so this clears the bit it just reported.
template<class Key, class KeyMapping, class Bits, class F>
constexpr auto walk_blocks_descending(Bits const& c, F& f)
        -> void
{
        using block_type      = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;

        auto const blocks = c.num_blocks();
        for (auto index = blocks - 1UZ; index < blocks; --index) {
                auto block = c[index];
                while (block != block_type{}) {
                        auto const offset = digits - 1UZ - static_cast<std::size_t>(countl_zero(block));
                        // A functor returning void has no exit to take, so its walk is compiled without one.
                        if constexpr (std::is_invocable_r_v<bool, F&, Key>) {
                                if (not f(decay_copy<Key>(KeyMapping::from_index((digits * index) + offset)))) {
                                        return;
                                }
                        } else {
                                f(decay_copy<Key>(KeyMapping::from_index((digits * index) + offset)));
                        }
                        block = static_cast<block_type>(block ^ shl(block_type{1}, offset));
                }
        }
}

// A mapping that closes the universe says how many keys it has, which an owner's width must then be.
template<class KeyMapping, class Key, std::size_t N>
concept admits_width = (not sized_bit_index_mapping<KeyMapping, Key>) or (KeyMapping::size == N);

// Position order is structural, so a comparator only picks a direction, with the key named or transparently.
template<class Compare, class Key>
concept key_direction = std::same_as<Compare, std::less<Key>> or std::same_as<Compare, std::less<>> or std::same_as<Compare, std::greater<Key>> or std::same_as<Compare, std::greater<>>;

// A comparator that lets a key of another type in, as [associative.reqmts.general]/180 has it.
template<class Compare>
concept transparent = requires { typename Compare::is_transparent; };

} // namespace set

template<bit_block_container_type Bits, storage Store = storage::owned, class Derived = void, class Key = std::size_t, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, class Compare = std::less<Key>>
class set_adaptor;

namespace set {

// The storage an owner has or a view's handle into another's, in a base public exactly where the owner is structural.
template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
using members_t = owner_members<
        adapted_bits<
                std::conditional_t<owns(Store), Bits, storage_ref_t<Bits>>,
                std::conditional_t<owns(Store), allocator_base_type<std::remove_const_t<Bits>, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>>, xstd::empty_base_type<>>,
                owns(Store) and std::remove_const_t<Bits>::is_structural>,
        std::remove_const_t<Bits>, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>, owns(Store)>;

// The positions an owner can hold where its type fixes them, its width or its capacity, else dynamic_extent.
template<class Bits, storage Store>
[[nodiscard]] consteval auto static_max_size() noexcept
        -> std::size_t
{
        using bits_type = std::remove_const_t<Bits>;
        if constexpr (owns(Store) and bits_type::extent != std::dynamic_extent) {
                return bits_type::extent;
        } else if constexpr (owns(Store) and bits_type::has_static_capacity) {
                return bits_type::static_capacity();
        } else {
                return std::dynamic_extent;
        }
}

// [container.reqmts]/57's max_size() as a constant, where the type fixes it; a set's size() counts its keys.
template<class Members, std::size_t N>
struct fixed_max_size : Members
{
        using Members::Members;

        static constexpr std::integral_constant<std::size_t, N> max_size = {};

        [[nodiscard]] friend auto operator==(fixed_max_size const&, fixed_max_size const&) -> bool = default;
};

// [container.reqmts]/57, distance(begin(), end()) for the largest container: every position set that names a key.
template<class Members, class Bits, storage Store, class Key, class KeyMapping>
struct run_time_max_size : Members
{
        using Members::Members;

        [[nodiscard]] constexpr auto max_size() const noexcept
                -> std::size_t
        {
                if constexpr ((std::remove_const_t<Bits>::extent != std::dynamic_extent)) {
                        return std::remove_const_t<Bits>::extent;
                } else if constexpr (owns(Store) and sized_bit_index_mapping<KeyMapping, Key>) {
                        return std::ranges::min(static_cast<std::size_t>(KeyMapping::size), this->m_bits.max_size());
                } else if constexpr (owns(Store)) {
                        return this->m_bits.max_size();
                } else {
                        return (*this->m_bits).size();
                }
        }

        [[nodiscard]] friend auto operator==(run_time_max_size const&, run_time_max_size const&) -> bool = default;
};

// The adaptor's base: its storage under max_size() as its column has it, which the adaptor does not declare.
template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
using sizes_t = std::conditional_t<
        static_max_size<Bits, Store>() != std::dynamic_extent,
        fixed_max_size<members_t<Bits, Store, Derived, Key, KeyMapping, Compare>, static_max_size<Bits, Store>()>,
        run_time_max_size<members_t<Bits, Store, Derived, Key, KeyMapping, Compare>, Bits, Store, Key, KeyMapping>>;

} // namespace set

template<bit_block_container_type Bits, storage Store, class Derived, class Key, bit_index_mapping<Key> KeyMapping, class Compare>
class set_adaptor : public set::sizes_t<Bits, Store, Derived, Key, KeyMapping, Compare>
{
        static_assert(set::key_direction<Compare, Key>);

        static constexpr bool is_owner      = owns(Store);
        static constexpr bool is_descending = std::same_as<Compare, std::greater<Key>> or std::same_as<Compare, std::greater<>>;

        // A zero width answers zero to every question: the exclusive scans need a position it has none to give.
        static constexpr bool is_zero_width = (Bits::extent == 0UZ);

        // An enumerator stands for the one key it is, except where the mapping reads every value as a mask of keys.
        static constexpr bool enumerator_is_one_key = std::is_enum_v<Key> and not bit_mask_mapping<KeyMapping, Key>;

        using bits_type = std::remove_const_t<Bits>;

        using members_type = set::sizes_t<Bits, Store, Derived, Key, KeyMapping, Compare>;
        using members_type::m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow.
        [[nodiscard]] constexpr auto bits(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return self.m_bits;
                } else {
                        return *self.m_bits;
                }
        }

        // Members rather than templates over Bits or KeyMapping, so ADL looks in the key's namespaces alone.
        template<class Value>
        class basic_reference;

        // A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through.
        template<class Value = Key>
        class basic_iterator
        {
                // Value exists only to put the key's namespaces among the associated ones: it is no second axis.
                static_assert(std::same_as<Value, Key>);

                storage_ptr_t<bits_type const> m_ptr{};
                std::size_t m_idx{};

                friend set_adaptor;

                friend class basic_reference<Value>;

                [[nodiscard]] constexpr basic_iterator(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                        : m_ptr(ptr)
                        , m_idx(idx)
                {
                        assert(m_ptr != nullptr);
                }

        public:
                using iterator_category = std::bidirectional_iterator_tag;
                using value_type        = Value;
                using difference_type   = std::ptrdiff_t;
                using pointer           = void;
                using reference         = basic_reference<Value>;

                [[nodiscard]] basic_iterator() = default;

                // A zero width has one position, so every iterator over it is the same one and every loop stops early.
                [[nodiscard]] friend constexpr auto operator==(basic_iterator lhs, basic_iterator rhs) noexcept
                        -> bool
                {
                        assert(lhs.m_ptr == rhs.m_ptr);
                        if constexpr (is_zero_width) {
                                return true;
                        } else {
                                return lhs.m_idx == rhs.m_idx;
                        }
                }

                [[nodiscard]] constexpr auto operator*() const noexcept
                        -> reference
                {
                        assert(m_ptr != nullptr);
                        return {m_ptr, m_idx};
                }

                constexpr auto operator++() noexcept
                        -> basic_iterator&
                {
                        assert(m_ptr != nullptr);
                        if constexpr (not is_zero_width) {
                                m_idx = set_adaptor::next_position(m_ptr, m_idx);
                        }
                        return *this;
                }

                constexpr auto operator--() noexcept
                        -> basic_iterator&
                {
                        assert(m_ptr != nullptr);
                        if constexpr (not is_zero_width) {
                                m_idx = set_adaptor::prev_position(m_ptr, m_idx);
                        }
                        return *this;
                }

                constexpr auto operator++(int) noexcept
                        -> basic_iterator
                {
                        auto nrv = *this;
                        ++*this;
                        return nrv;
                }

                constexpr auto operator--(int) noexcept
                        -> basic_iterator
                {
                        auto nrv = *this;
                        --*this;
                        return nrv;
                }
        };

        // The key at a position, arriving by conversion; & hands the iterator back, so the pair round-trips.
        template<class Value = Key>
        class basic_reference
        {
                // Value exists only to put the key's namespaces among the associated ones: it is no second axis.
                static_assert(std::same_as<Value, Key>);

                storage_ptr_t<bits_type const> m_ptr;
                std::size_t m_idx;

                friend set_adaptor;

                friend class basic_iterator<Value>;

                [[nodiscard]] constexpr basic_reference(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                        : m_ptr(ptr)
                        , m_idx(idx)
                {
                        assert(m_ptr != nullptr);
                }

        public:
                using value_type   = Value;
                using iterator     = basic_iterator<Value>;
                using adaptor_type = set_adaptor;

                // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is.
                basic_reference(basic_reference const&)                    = default;
                auto operator=(basic_reference const&) -> basic_reference& = delete;

                [[nodiscard]] constexpr auto operator&() const noexcept
                        -> iterator
                {
                        return {m_ptr, m_idx};
                }

                // The one conversion: comparisons are the key's own through it.
                [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
                {
                        return set_adaptor::key_at(m_idx);
                }

                // What this proxy prints as, said once: our std::formatter calls it unqualified, fmt finds it by ADL.
                [[nodiscard]] friend constexpr auto format_as(basic_reference ref) noexcept
                        -> value_type
                {
                        return ref;
                }
        };

        // The walk's step up, in the comparator's direction; a zero width has no position for a scan to start from.
        [[nodiscard]] static constexpr auto next_position(storage_ptr_t<bits_type const> const& ptr, std::size_t n) noexcept
                -> std::size_t
                requires (not is_zero_width)
        {
                assert(n < ptr->size());
                if constexpr (is_descending) {
                        return ptr->total_find_prev(n);
                } else {
                        return ptr->exclusive_find_next(n);
                }
        }

        // Descending, the end is size() as well, so stepping back from it is a step up to the lowest position.
        [[nodiscard]] static constexpr auto prev_position(storage_ptr_t<bits_type const> const& ptr, std::size_t n) noexcept
                -> std::size_t
                requires (not is_zero_width)
        {
                if constexpr (not is_descending) {
                        assert(ptr->find_first() < n);
                        return ptr->exclusive_find_prev(n);
                } else if (n == ptr->size()) {
                        assert(ptr->find_first() < ptr->size());
                        return ptr->find_first();
                } else {
                        assert(ptr->exclusive_find_next(n) < ptr->size());
                        return ptr->exclusive_find_next(n);
                }
        }

        [[nodiscard]] static constexpr auto key_at(std::size_t n) noexcept
                -> Key
        {
                return KeyMapping::from_index(n);
        }

        // The container needs constraints only the vehicle can name; [class.friend]/3 ignores the void a view passes.
        friend Derived;

        // A view refers into this owner's storage, and only a reading that can view it is named.
        template<bit_block_container_type, storage, class, class OtherKey, bit_index_mapping<OtherKey>, class>
        friend class set_adaptor;

        // The free functions over every reading, bit_convert among them, reach the storage through this one door.
        friend struct storage_access;

        // The value as std::set<Key, Compare> hashes it, a view as its owner: the keys in order, then their count.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const& pr, Hash& h, Flavor const& f, set_adaptor const* v) noexcept
                -> void
        {
                hash_append_keys(pr, h, f, *v);
        }

public:
        // A vehicle used directly is its own container, which is what a view is.
        using derived_type = std::conditional_t<std::is_void_v<Derived>, set_adaptor, Derived>;

        // What a trait asks of this vehicle, every container built on it answering alike.
        using adaptor_type = set_adaptor;
        using reads_as     = set_reading_tag;
        using adapted_type = Bits;

        static constexpr bool owns_storage     = is_owner;
        static constexpr bool has_static_width = (Bits::extent != std::dynamic_extent);

        // types
        using key_type               = Key;
        using key_compare            = Compare;
        using value_type             = key_type;
        using value_compare          = key_compare;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = basic_reference<>;
        using const_reference        = reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = basic_iterator<>;
        using const_iterator         = iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // Not in [set.overview]: how a key maps to a position.
        using key_mapping_type = KeyMapping;

private:
        // An allocator argument as [container.alloc.reqmts] takes it: converting, and only where the storage has one.
        static constexpr bool is_allocator_aware = allocator_aware<std::remove_const_t<Bits>>;

        using allocator_param = allocator_param_t<std::remove_const_t<Bits>>;

public:
        // construct/copy/destroy; an owner is built the way std::set is, a view only from what it views.
        [[nodiscard]] set_adaptor() noexcept(std::is_nothrow_default_constructible_v<Bits>)
                requires is_owner
        = default;

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last)
        {
                insert(first, last);
        }

        template<std::ranges::input_range R>
                requires is_owner and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg)
        {
                insert(std::ranges::begin(rg), std::ranges::end(rg));
        }

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il)
                requires is_owner
        {
                insert(il.begin(), il.end());
        }

        // [set.cons]'s comparator arguments, taken and dropped: each key_compare a set reading admits has no state.
        [[nodiscard]] constexpr explicit set_adaptor(key_compare const& /* comp */) noexcept(std::is_nothrow_default_constructible_v<Bits>)
                requires is_owner
        {}

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last, key_compare const& /* comp */)
                : set_adaptor(first, last)
        {}

        template<std::ranges::input_range R>
                requires is_owner and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg, key_compare const& /* comp */)
                : set_adaptor(std::from_range, std::forward<R>(rg))
        {}

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il, key_compare const& /* comp */)
                requires is_owner
                : set_adaptor(il)
        {}

        // [set.cons]'s allocator arguments: converting, and offered only where the storage has an allocator.
        [[nodiscard]] constexpr explicit set_adaptor(allocator_param const& alloc) noexcept(std::is_nothrow_constructible_v<Bits, allocator_param const&>)
                requires is_owner and is_allocator_aware
                : members_type(std::in_place, alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(key_compare const& /* comp */, allocator_param const& alloc) noexcept(std::is_nothrow_constructible_v<Bits, allocator_param const&>)
                requires is_owner and is_allocator_aware
                : members_type(std::in_place, alloc)
        {}

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and is_allocator_aware and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last, allocator_param const& alloc)
                : members_type(std::in_place, alloc)
        {
                insert(first, last);
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and is_allocator_aware and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last, key_compare const& /* comp */, allocator_param const& alloc)
                : set_adaptor(first, last, alloc)
        {}

        template<std::ranges::input_range R>
                requires is_owner and is_allocator_aware and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg, allocator_param const& alloc)
                : set_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        template<std::ranges::input_range R>
                requires is_owner and is_allocator_aware and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg, key_compare const& /* comp */, allocator_param const& alloc)
                : set_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il, allocator_param const& alloc)
                requires is_owner and is_allocator_aware
                : set_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il, key_compare const& /* comp */, allocator_param const& alloc)
                requires is_owner and is_allocator_aware
                : set_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(set_adaptor const& other, allocator_param const& alloc)
                requires is_owner and is_allocator_aware
                : members_type(std::in_place, other.m_bits, alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(set_adaptor&& other, allocator_param const& alloc)
                requires is_owner and is_allocator_aware
                : members_type(std::in_place, std::move(other.m_bits), alloc)
        {}

        // flat_set's adopting constructor at a run-time width: the blocks move in, every bit of them a position.
        [[nodiscard]] constexpr set_adaptor(xstd::from_blocks_t, bits_type::block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<typename bits_type::block_container_type>)
                requires is_owner and bits_type::has_stored_size
                : members_type(std::in_place, xstd::from_blocks, std::move(blocks))
        {}

        [[nodiscard]] constexpr set_adaptor(xstd::from_blocks_t, bits_type::block_container_type blocks, allocator_param const& alloc)
                requires is_owner and bits_type::has_stored_size and is_allocator_aware
                : members_type(std::in_place, xstd::from_blocks, std::move(blocks), alloc)
        {}

        // Blocks that are bit storage, read as this set's positions; the tag says the blocks are bits and not keys.
        template<class OtherBits>
                requires is_owner and xstd::bit_blocks<OtherBits> and Bits::template
        exchanges_bits<OtherBits> [[nodiscard]] constexpr set_adaptor(xstd::from_blocks_t, OtherBits const& b) noexcept
        {
                m_bits.assign_bits(b);
        }

        [[nodiscard]] constexpr explicit set_adaptor(Bits& c) noexcept
                requires (not is_owner)
                : members_type(std::in_place, &c)
        {}

        // Blocks handed straight over, held as the storage that borrows them, as std::views::all holds a view.
        template<class Blocks>
                requires (not is_owner) and (borrowable_block<Blocks &&> or borrowable_blocks<Blocks &&>) and std::same_as<borrowed_bits_t<Blocks&&>, Bits>
        [[nodiscard]] constexpr explicit set_adaptor(Blocks&& blocks) noexcept
                : members_type(std::in_place, borrow_bits(std::forward<Blocks>(blocks)))
        {}

        // A view over an owner is a view over the storage it wraps; implicit, claiming nothing the owner lacks.
        template<owner_of<Bits, set_reading_tag> Owner>
        [[nodiscard]] constexpr explicit(false) set_adaptor(Owner& c) noexcept // NOLINT(misc-explicit-constructor)
                requires (not is_owner)
                : members_type(std::in_place, &c.m_bits)
        {}

        // span's qualification conversion: a view of mutable bits is implicitly a view of the same bits as const.
        template<class OtherDerived>
                requires (not is_owner) and std::is_const_v<Bits>
        [[nodiscard]] constexpr explicit(false) set_adaptor(set_adaptor<std::remove_const_t<Bits>, Store, OtherDerived, Key, KeyMapping, Compare> const& other) noexcept // NOLINT(misc-explicit-constructor)
                : set_adaptor(other.bits())
        {}

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [set] and [vector] return here.
        constexpr auto operator=(std::initializer_list<value_type> il)
                -> derived_type&
                requires is_owner
        {
                clear();
                insert(il.begin(), il.end());
                return self(); // NOLINT(misc-unconventional-assign-operator)
        }

        // A static owner's equality is its one member's: every instance carries the same width.
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
                requires std::equality_comparable<bits_type> and is_owner and has_static_width
        {
                return x.bits() == y.bits();
        }

        // Everything else: the storage's set equality, which answers at any two widths and over lent blocks.
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
                requires (not(is_owner and has_static_width))
        {
                return set_equal(x.bits(), y.bits());
        }

        // The blockwise set order; [associative.reqmts]' order over descending keys is the masks' as unsigned numbers.
        [[nodiscard]] friend constexpr auto operator<=>(set_adaptor const& x, set_adaptor const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (is_descending) {
                        return numeric_three_way(x.bits(), y.bits());
                } else {
                        return three_way<set_reading_tag>(x.bits(), y.bits());
                }
        }

        // iterators; one type for both, this reading being read-only through its proxy, and size() the end either way.
        [[nodiscard]] constexpr auto begin() const noexcept
                -> const_iterator
        {
                if constexpr (is_descending) {
                        return {&bits(), bits().total_find_prev(bits().size())};
                } else {
                        return {&bits(), bits().find_first()};
                }
        }

        [[nodiscard]] constexpr auto end() const noexcept
                -> const_iterator
        {
                return {&bits(), bits().size()};
        }

        [[nodiscard]] constexpr auto rbegin() const noexcept
                -> const_reverse_iterator
        {
                return std::make_reverse_iterator(end());
        }

        [[nodiscard]] constexpr auto rend() const noexcept
                -> const_reverse_iterator
        {
                return std::make_reverse_iterator(begin());
        }

        [[nodiscard]] constexpr auto cbegin() const noexcept
                -> const_iterator
        {
                return begin();
        }

        [[nodiscard]] constexpr auto cend() const noexcept
                -> const_iterator
        {
                return end();
        }

        [[nodiscard]] constexpr auto crbegin() const noexcept
                -> const_reverse_iterator
        {
                return rbegin();
        }

        [[nodiscard]] constexpr auto crend() const noexcept
                -> const_reverse_iterator
        {
                return rend();
        }

        // The set reading a block at a time in the iteration order, which is what an iterator cannot be.
        template<class F>
                requires std::invocable<F&, key_type>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                if constexpr (is_descending) {
                        set::walk_blocks_descending<Key, KeyMapping>(self.bits(), f);
                } else {
                        set::walk_blocks_ascending<Key, KeyMapping>(self.bits(), f);
                }
        }

        // The mirror, the last key in the iteration order first.
        template<class F>
                requires std::invocable<F&, key_type>
        constexpr auto for_each_reverse(this auto&& self, F f)
                -> void
        {
                if constexpr (is_descending) {
                        set::walk_blocks_ascending<Key, KeyMapping>(self.bits(), f);
                } else {
                        set::walk_blocks_descending<Key, KeyMapping>(self.bits(), f);
                }
        }

        // capacity; a bitset's count() is a set's size(), and max_size() is the positions there are to hold.
        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
        {
                return begin() == end();
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return bits().count();
        }

        using members_type::max_size;

        // element access, both with a non-empty set as their precondition.
        [[nodiscard]] constexpr auto front() const noexcept
                -> const_reference
        {
                return *begin();
        }

        // A zero width has no position to scan back from, and exclusive_find_prev asserts there.
        [[nodiscard]] constexpr auto back() const noexcept
                -> const_reference
        {
                if constexpr (is_zero_width) {
                        return {&bits(), 0UZ};
                } else if constexpr (is_descending) {
                        return {&bits(), bits().find_first()};
                } else {
                        return {&bits(), bits().exclusive_find_prev(bits().size())};
                }
        }

        // modifiers; each writes through the storage, so each exists exactly where the storage lets this handle write.
        template<class... Args>
        constexpr auto emplace(this auto&& self, Args&&... args)
                -> std::pair<iterator, bool>
                requires (sizeof...(args) <= 1) and requires { self.bits().growing_insert(KeyMapping::to_index(value_type(std::forward<Args>(args)...))); }
        {
                return self.do_insert(KeyMapping::to_index(value_type(std::forward<Args>(args)...)));
        }

        template<class... Args>
        constexpr auto emplace_hint(this auto&& self, const_iterator position, Args&&... args)
                -> iterator
                requires (sizeof...(args) <= 1) and requires { self.bits().growing_insert(KeyMapping::to_index(value_type(std::forward<Args>(args)...))); }
        {
                return self.do_insert(position, KeyMapping::to_index(value_type(std::forward<Args>(args)...)));
        }

        // [set]'s two overloads by value: a key is kept only as its position, so there is nothing to move.
        constexpr auto insert(this auto&& self, value_type x)
                -> std::pair<iterator, bool>
                requires requires { self.bits().growing_insert(KeyMapping::to_index(x)); }
        {
                return self.do_insert(KeyMapping::to_index(x));
        }

        constexpr auto insert(this auto&& self, const_iterator position, value_type x)
                -> iterator
                requires requires { self.bits().growing_insert(KeyMapping::to_index(x)); }
        {
                return self.do_insert(position, KeyMapping::to_index(x));
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr auto insert(this auto&& self, I first, S last)
                -> void
                requires std::constructible_from<value_type, std::iter_reference_t<I>> and requires { self.bits().growing_insert(KeyMapping::to_index(static_cast<value_type>(*first))); }
        {
                for (; first != last; ++first) {
                        auto const x = KeyMapping::to_index(static_cast<value_type>(*first));
                        self.guard_key(x);
                        self.bits().growing_insert(x);
                }
        }

        // Ranged insertion has tiers, as the sequence reading's append_range does.
        template<std::ranges::input_range R>
        constexpr auto insert_range(this auto&& self, R&& rg)
                -> void
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>> and requires { self.bits().growing_insert(KeyMapping::to_index(static_cast<value_type>(*std::ranges::begin(rg)))); }
        {
                if constexpr (requires { self |= rg; }) {
                        // Tier one: another set over the same storage, which is a union done block-wise.
                        self |= rg;
                } else if constexpr (xstd::specialization_of<std::remove_cvref_t<R>, std::ranges::iota_view> and std::same_as<KeyMapping, bit_key_mapping<std::size_t>> and requires (std::size_t pos, std::size_t len) { self.bits().set(pos, len, true); }) {
                        // Tier two: consecutive identity keys, the first and last blocks masked, the rest whole.
                        if (not std::ranges::empty(rg)) {
                                auto const lo  = static_cast<value_type>(*std::ranges::begin(rg));
                                auto const len = static_cast<std::size_t>(std::ranges::distance(rg));
                                // The last position first, so a growable storage is wide enough.
                                auto const hi = bits_type::width_sum(lo, len - 1UZ);
                                self.guard_key(hi);
                                self.bits().growing_insert(hi);
                                self.bits().set(lo, len, true);
                        }
                } else {
                        self.insert(std::ranges::begin(rg), std::ranges::end(rg));
                }
        }

        constexpr auto insert(this auto&& self, std::initializer_list<value_type> ilist)
                -> void
                requires requires { self.bits().growing_insert(KeyMapping::to_index(*ilist.begin())); }
        {
                self.insert(ilist.begin(), ilist.end());
        }

        constexpr auto fill(this auto&& self) noexcept
                -> void
                requires requires { self.bits().fill(true); }
        {
                self.bits().fill(true);
        }

        // The successor first: exclusive_find_next never reads the position it steps from.
        constexpr auto erase(this auto&& self, const_iterator position) noexcept
                -> iterator
                requires requires (std::size_t pos) { self.bits().assign(pos, false); }
        {
                assert(position != self.end());
                auto nrv = position;
                ++nrv;
                self.bits().assign(KeyMapping::to_index(*position), false);
                return nrv;
        }

        // Total over key_type, as std::set's is: an absent key is the no-op returning zero.
        constexpr auto erase(this auto&& self, key_type const& x) noexcept
                -> size_type
                requires requires { self.bits().assign(KeyMapping::to_index(x), false); }
        {
                if (not self.contains(x)) {
                        return 0UZ;
                }
                self.bits().assign(KeyMapping::to_index(x), false);
                return 1UZ;
        }

        constexpr auto erase(this auto&& self, const_iterator first, const_iterator last) noexcept
                -> iterator
                requires requires (std::size_t pos) { self.bits().assign(pos, false); }
        {
                // A range, not two positions: reversed, the walk steps past last into a scan nothing answers.
                assert(is_descending ? (last == self.end() or KeyMapping::to_index(*last) <= KeyMapping::to_index(*first)) : KeyMapping::to_index(*first) <= KeyMapping::to_index(*last));
                while (first != last) {
                        self.bits().assign(KeyMapping::to_index(*first++), false);
                }
                return last;
        }

        // The blocks a from_blocks constructor takes, and what an owner's replace and extract trade in.
        using block_container_type = Bits::block_container_type;

        constexpr auto clear(this auto&& self) noexcept
                -> void
                requires requires { self.bits().fill(false); }
        {
                self.bits().fill(false);
        }

        // Bulk on the storage's spelling; union and symmetric difference grow, the other two do not.
        constexpr auto operator&=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.bits() &= other.bits(); }
        {
                self.bits() &= other.bits();
                return self;
        }

        constexpr auto operator|=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.bits().grow_to_admit(other.bits()); self.bits() |= other.bits(); }
        {
                self.bits().grow_to_admit(other.bits());
                self.bits() |= other.bits();
                return self;
        }

        constexpr auto operator^=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.bits().grow_to_admit(other.bits()); self.bits() ^= other.bits(); }
        {
                self.bits().grow_to_admit(other.bits());
                self.bits() ^= other.bits();
                return self;
        }

        constexpr auto operator-=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.bits() -= other.bits(); }
        {
                self.bits() -= other.bits();
                return self;
        }

        // A view of ours takes any other set over this storage and keys, owner or view, by the storage's spelling.
        template<class OtherBits, storage OtherStore, class OtherDerived>
        constexpr auto operator&=(this auto&& self, set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare> const& other) noexcept
                -> auto&
                requires (not is_owner) and std::same_as<std::remove_const_t<OtherBits>, std::remove_const_t<Bits>> and (not std::same_as<set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare>, set_adaptor>) and requires { self.bits() &= other.bits(); }
        {
                self.bits() &= other.bits();
                return self;
        }

        template<class OtherBits, storage OtherStore, class OtherDerived>
        constexpr auto operator|=(this auto&& self, set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare> const& other) noexcept(has_static_width)
                -> auto&
                requires (not is_owner) and std::same_as<std::remove_const_t<OtherBits>, std::remove_const_t<Bits>> and (not std::same_as<set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare>, set_adaptor>) and requires { self.bits().grow_to_admit(other.bits()); self.bits() |= other.bits(); }
        {
                self.bits().grow_to_admit(other.bits());
                self.bits() |= other.bits();
                return self;
        }

        template<class OtherBits, storage OtherStore, class OtherDerived>
        constexpr auto operator^=(this auto&& self, set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare> const& other) noexcept(has_static_width)
                -> auto&
                requires (not is_owner) and std::same_as<std::remove_const_t<OtherBits>, std::remove_const_t<Bits>> and (not std::same_as<set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare>, set_adaptor>) and requires { self.bits().grow_to_admit(other.bits()); self.bits() ^= other.bits(); }
        {
                self.bits().grow_to_admit(other.bits());
                self.bits() ^= other.bits();
                return self;
        }

        template<class OtherBits, storage OtherStore, class OtherDerived>
        constexpr auto operator-=(this auto&& self, set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare> const& other) noexcept
                -> auto&
                requires (not is_owner) and std::same_as<std::remove_const_t<OtherBits>, std::remove_const_t<Bits>> and (not std::same_as<set_adaptor<OtherBits, OtherStore, OtherDerived, Key, KeyMapping, Compare>, set_adaptor>) and requires { self.bits() -= other.bits(); }
        {
                self.bits() -= other.bits();
                return self;
        }

        // An enumerator is the one-element set holding it, so it meets a set without an operator on the enumeration.
        constexpr auto operator&=(this auto&& self, key_type x) noexcept
                -> auto&
                requires enumerator_is_one_key and requires { self.clear(); self.bits().assign(KeyMapping::to_index(x), true); }
        {
                auto const kept = self.contains(x);
                self.clear();
                if (kept) {
                        self.bits().assign(KeyMapping::to_index(x), true);
                }
                return self;
        }

        // Not noexcept: an enumerator outside the listed values throws here, as insert does.
        constexpr auto operator|=(this auto&& self, key_type x)
                -> auto&
                requires enumerator_is_one_key and requires { self.insert(x); }
        {
                static_cast<void>(self.insert(x));
                return self;
        }

        constexpr auto operator^=(this auto&& self, key_type x)
                -> auto&
                requires enumerator_is_one_key and requires { self.complement(x); }
        {
                self.complement(x);
                return self;
        }

        constexpr auto operator-=(this auto&& self, key_type x) noexcept
                -> auto&
                requires enumerator_is_one_key and requires { self.erase(x); }
        {
                static_cast<void>(self.erase(x));
                return self;
        }

        // Hidden friends, so two enumerators never reach them; they copy, and so are the owner's alone.
        [[nodiscard]] friend constexpr auto operator&(set_adaptor const& lhs, key_type rhs) noexcept(has_static_width)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                auto nrv = static_cast<derived_type const&>(lhs);
                nrv &= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator&(key_type lhs, set_adaptor const& rhs) noexcept(has_static_width)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                return rhs & lhs;
        }

        [[nodiscard]] friend constexpr auto operator|(set_adaptor const& lhs, key_type rhs)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                auto nrv = static_cast<derived_type const&>(lhs);
                nrv |= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator|(key_type lhs, set_adaptor const& rhs)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                return rhs | lhs;
        }

        [[nodiscard]] friend constexpr auto operator^(set_adaptor const& lhs, key_type rhs)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                auto nrv = static_cast<derived_type const&>(lhs);
                nrv ^= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator^(key_type lhs, set_adaptor const& rhs)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                return rhs ^ lhs;
        }

        [[nodiscard]] friend constexpr auto operator-(set_adaptor const& lhs, key_type rhs) noexcept(has_static_width)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                auto nrv = static_cast<derived_type const&>(lhs);
                nrv -= rhs;
                return nrv;
        }

        // The one-element set less the other: toggling the enumerator in a copy, then keeping only it.
        [[nodiscard]] friend constexpr auto operator-(key_type lhs, set_adaptor const& rhs)
                -> derived_type
                requires is_owner and enumerator_is_one_key
        {
                auto nrv = static_cast<derived_type const&>(rhs);
                nrv ^= lhs;
                nrv &= lhs;
                return nrv;
        }

        // The shifts translate the set and keep the keys below max_size(); a run-time width grows towards it first.
        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept(has_static_width or bits_type::has_static_capacity)
                -> auto&
                requires requires { self.bits() <<= n; } and (has_static_width or requires { self.bits().resize(n); })
        {
                if constexpr (has_static_width) {
                        // The storage's shift has n < size() as its precondition; a constant n folds the test away.
                        if (n < self.bits().size()) {
                                self.bits() <<= n;
                        } else {
                                self.bits().fill(false);
                        }
                } else if constexpr (bits_type::has_zero_capacity) {
                        // A capacity of nought holds no element, so there is nothing to translate.
                } else if (auto const lowest = self.bits().find_first(), width = self.bits().size(), top = self.max_size(); lowest == width or n >= top - lowest) {
                        // No key lands below max_size(): the set empties at its width and asks for no growth.
                        self.bits().fill(false);
                } else {
                        // width + n saturates and stops at max_size(); the storage's shift drops what passes the width.
                        self.bits().resize(std::ranges::min(bits_type::width_sum(width, n), top));
                        self.bits() <<= n;
                }
                return self;
        }

        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept
                -> auto&
                requires requires { self.bits() >>= n; }
        {
                if (n < self.bits().size()) {
                        self.bits() >>= n;
                } else {
                        self.bits().fill(false);
                }
                return self;
        }

        // observers
        [[nodiscard]] constexpr auto key_comp() const noexcept
                -> key_compare
        {
                return {};
        }

        [[nodiscard]] constexpr auto value_comp() const noexcept
                -> value_compare
        {
                return {};
        }

        // set operations, total over key_type as std::set's are; is_key and the width are the guards, test() the read.
        [[nodiscard]] constexpr auto contains(key_type const& x) const noexcept
                -> bool
        {
                if (not detail::is_key<KeyMapping>(x)) {
                        return false;
                }
                auto const pos = KeyMapping::to_index(x);
                return pos < bits().size() and bits().test(pos);
        }

        [[nodiscard]] constexpr auto count(key_type const& x) const noexcept
                -> size_type
        {
                return contains(x);
        }

        [[nodiscard]] constexpr auto find(key_type const& x) const noexcept
                -> const_iterator
        {
                return contains(x) ? const_iterator{&bits(), KeyMapping::to_index(x)} : end();
        }

        // The first element key_compare puts no earlier than x; ascending, stepping from x - 1 would underflow at zero.
        [[nodiscard]] constexpr auto lower_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                if constexpr (orders_non_keys) {
                        if (not detail::is_key<KeyMapping>(x)) {
                                return bound_between(x);
                        }
                }
                if constexpr (is_descending) {
                        // The highest position not above x's, which is the scan below the one past it.
                        auto const pos = KeyMapping::to_index(x);
                        return {&bits(), bits().total_find_prev(pos < bits().size() ? pos + 1UZ : bits().size())};
                } else {
                        return contains(x) ? const_iterator{&bits(), KeyMapping::to_index(x)} : upper_bound(x);
                }
        }

        [[nodiscard]] constexpr auto upper_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                if constexpr (orders_non_keys) {
                        if (not detail::is_key<KeyMapping>(x)) {
                                return bound_between(x);
                        }
                }
                auto const pos = KeyMapping::to_index(x);
                if constexpr (is_descending) {
                        return {&bits(), bits().total_find_prev(std::ranges::min(pos, bits().size()))};
                } else {
                        if (pos >= bits().size()) {
                                return end();
                        }
                        return {&bits(), bits().exclusive_find_next(pos)};
                }
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) const noexcept
                -> std::pair<const_iterator, const_iterator>
        {
                return {lower_bound(x), upper_bound(x)};
        }

        // The heterogeneous forms, over the positions whose keys are equivalent to x under key_compare.
        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto contains(K const& x) const
                -> bool
        {
                auto const [lo, hi] = equivalent_positions(x);
                return bits().inclusive_find_next(lo) < hi;
        }

        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto count(K const& x) const
                -> size_type
        {
                auto const [lo, hi] = equivalent_positions(x);
                auto n              = 0UZ;
                for (auto pos = bits().inclusive_find_next(lo); pos < hi; pos = bits().exclusive_find_next(pos)) {
                        ++n;
                }
                return n;
        }

        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto find(K const& x) const
                -> const_iterator
        {
                auto const [lo, hi] = equivalent_positions(x);
                auto const pos      = bits().inclusive_find_next(lo);
                return pos < hi ? const_iterator{&bits(), pos} : end();
        }

        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto lower_bound(K const& x) const
                -> const_iterator
        {
                auto const [lo, hi] = equivalent_positions(x);
                if constexpr (is_descending) {
                        return {&bits(), bits().total_find_prev(hi)};
                } else {
                        return {&bits(), bits().inclusive_find_next(lo)};
                }
        }

        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto upper_bound(K const& x) const
                -> const_iterator
        {
                auto const [lo, hi] = equivalent_positions(x);
                if constexpr (is_descending) {
                        return {&bits(), bits().total_find_prev(lo)};
                } else {
                        return {&bits(), bits().inclusive_find_next(hi)};
                }
        }

        template<class K>
                requires set::transparent<key_compare>
        [[nodiscard]] constexpr auto equal_range(K const& x) const
                -> std::pair<const_iterator, const_iterator>
        {
                return {lower_bound(x), upper_bound(x)};
        }

        // [associative.reqmts.general]/122; a key_type is left to its own overload, which MSVC cannot rank above this.
        template<class K>
                requires set::transparent<key_compare> and (not std::same_as<std::remove_cvref_t<K>, key_type>) and (not std::convertible_to<K &&, iterator>) and (not std::convertible_to<K &&, const_iterator>)
                                                                                                                                                          constexpr auto erase(this auto&& self, K&& x) -> size_type
                                 requires requires (std::size_t pos, std::size_t len) { self.bits().set(pos, len, false); }
        {
                auto const erased   = self.count(x);
                auto const [lo, hi] = self.equivalent_positions(x);
                self.bits().set(lo, hi - lo, false);
                return erased;
        }

private:
        // Toggling one key, growing where insert grows; not noexcept, since growing allocates.
        constexpr auto complement(this auto&& self, value_type x)
                -> void
                requires requires { self.bits().assign(KeyMapping::to_index(x), true); }
        {
                auto const pos = KeyMapping::to_index(x);
                self.guard_key(pos);
                if constexpr (not has_static_width and requires { self.bits().growing_insert(pos); }) {
                        if (pos >= self.bits().size()) {
                                static_cast<void>(self.bits().growing_insert(pos));
                                return;
                        }
                }
                assert(pos < self.bits().size());
                self.bits().assign(pos, not self.bits().test(pos));
        }

        // The whole-set complement, at a static width alone: complementing needs a universe, which N is.
        constexpr auto complement(this auto&& self) noexcept
                -> void
                requires has_static_width and requires { self.bits().flip(); }
        {
                self.bits().flip();
        }

        // A key type with no order of its own, a std::bitset, has no place between two keys for a value that is no key.
        static constexpr bool orders_non_keys = std::totally_ordered<key_type>;

        // A value that is no key falls between two keys, where both its bounds are, bisected under key_compare.
        [[nodiscard]] constexpr auto bound_between(key_type const& x) const
                -> const_iterator
        {
                auto const lo = equivalent_positions(x).first;
                if constexpr (is_descending) {
                        return {&bits(), bits().total_find_prev(lo)};
                } else {
                        return {&bits(), bits().inclusive_find_next(lo)};
                }
        }

        // The positions whose keys are equivalent to x, bisected under key_compare, the keys being in position order.
        template<class K>
        [[nodiscard]] constexpr auto equivalent_positions(K const& x) const
                -> std::pair<std::size_t, std::size_t>
        {
                auto const comp      = key_compare();
                auto const positions = std::views::iota(0UZ, bits().size());
                // Descending, a key orders before x exactly where comp(x, key) holds.
                auto const below = [&](std::size_t pos) -> bool {
                        if constexpr (is_descending) {
                                return comp(x, KeyMapping::from_index(pos));
                        } else {
                                return comp(KeyMapping::from_index(pos), x);
                        }
                };
                auto const not_above = [&](std::size_t pos) -> bool {
                        if constexpr (is_descending) {
                                return not comp(KeyMapping::from_index(pos), x);
                        } else {
                                return not comp(x, KeyMapping::from_index(pos));
                        }
                };
                auto const first = std::ranges::begin(positions);
                auto const lo    = std::ranges::partition_point(positions, below);
                auto const hi    = std::ranges::partition_point(std::ranges::subrange(lo, std::ranges::end(positions)), not_above);
                return {static_cast<std::size_t>(lo - first), static_cast<std::size_t>(hi - first)};
        }

        // The container built on this vehicle, which is what its value-returning operators hand back.
        [[nodiscard]] constexpr auto self() noexcept
                -> derived_type&
        {
                return static_cast<derived_type&>(*this);
        }

        // Where a write is refused: past a closed universe at any width, and past what the storage can ever hold.
        constexpr auto guard_key(std::size_t x) const
                -> void
        {
                if constexpr (has_static_width) {
                        if (x >= max_size()) {
                                throw out_of_range(x);
                        }
                } else {
                        // A value that is no key ranks at or past the universe's size, as at a static width.
                        if constexpr (sized_bit_index_mapping<KeyMapping, Key>) {
                                if (x >= static_cast<std::size_t>(KeyMapping::size)) {
                                        throw out_of_range(x);
                                }
                        }
                        // Past what a heap can count is length_error here, and past a capacity bad_alloc on growth.
                        static_cast<void>(bits_type::check_width(bits_type::width_sum(x, 1UZ)));
                }
        }

        [[nodiscard]] constexpr auto out_of_range(std::size_t x, std::source_location const& loc = std::source_location::current()) const
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘x‘ is no key this set can hold [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), x, max_size()
                        )
                );
        }

        // growing_insert reports whether the bit was new, so one walk answers where two did.
        constexpr auto do_insert(this auto&& self, std::size_t x)
                -> std::pair<iterator, bool>
        {
                self.guard_key(x);
                auto const inserted = self.bits().growing_insert(x);
                return {{&self.bits(), x}, inserted};
        }

        constexpr auto do_insert(this auto&& self, const_iterator, std::size_t x)
                -> iterator
        {
                self.guard_key(x);
                self.bits().growing_insert(x);
                return {&self.bits(), x};
        }
};

// A proxy some set_adaptor hands out, recognized through the adaptor it names: no deduction reaches into a member.
template<class R>
concept set_reference = reads<typename R::adaptor_type, set_reading_tag> and std::same_as<R, typename R::adaptor_type::reference>;

// What a set owner wraps, so that a view over it names the same storage and reading.
template<class Bits, class Derived, class Key, class KeyMapping, class Compare>
struct owned_storage<set_adaptor<Bits, storage::owned, Derived, Key, KeyMapping, Compare>>
{
        using bits_type = Bits;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the clause needs them.

// 23.4.6.3 Erasure                                                [set.erasure]
template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare, class Predicate>
constexpr auto erase_if(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>& c, Predicate pred)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::size_type
{
        auto const original_size = c.size();
        for (auto i = c.begin(), last = c.end(); i != last;) {
                if (pred(*i)) {
                        i = c.erase(i);
                } else {
                        ++i;
                }
        }
        return original_size - c.size();
}

// The non-member forms copy, so they are the owner's alone: a copied view would write through.
template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator~(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width and requires (Bits b) { b.flip(); }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        storage_access::bits(nrv).flip();
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator&(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& rhs) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c &= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv &= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator|(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& rhs) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c |= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv |= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator^(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& rhs) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c ^= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv ^= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator-(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& rhs) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c -= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv -= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator<<(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c <<= n; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv <<= n;
        return nrv;
}

template<class Bits, storage Store, class Derived, class Key, class KeyMapping, class Compare>
[[nodiscard]] constexpr auto operator>>(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::has_static_width)
        -> set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> c) { c >>= n; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>::derived_type const&>(lhs);
        nrv >>= n;
        return nrv;
}

// NOLINTEND(readability-redundant-parentheses)

} // namespace xstd::bits::detail

namespace boost::container_hash {

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload.
template<class Bits, xstd::bits::detail::storage Store, class Derived, class Key, class KeyMapping, class Compare>
struct is_range<xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

// std::format over the containers, which prints the key as the key's own formatter does.
template<xstd::bits::detail::set_reference R, class CharT>
struct formatter<R, CharT> : formatter<typename R::value_type, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(R ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend.
                return formatter<typename R::value_type, CharT>::format(format_as(ref), ctx);
        }
};

// Owned or viewed, as std::string_view hashes and std::set does not: the bits, by bit_hasher.
template<class Bits, xstd::bits::detail::storage Store, class Derived, class Key, class KeyMapping, class Compare>
struct hash<xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bits::detail::set_adaptor<Bits, Store, Derived, Key, KeyMapping, Compare> const& v) const noexcept
                -> std::size_t
        {
                return xstd::bit_hasher<boost::hash2::xxhash_64>()(v);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.

namespace std::ranges {

// A view is a std::ranges::view outright and borrowed, its iterators pointing at the storage.
template<class Bits>
inline constexpr bool enable_view<xstd::bits::detail::set_adaptor<Bits, xstd::bits::detail::storage::borrowed>> = true;

template<class Bits>
inline constexpr bool enable_borrowed_range<xstd::bits::detail::set_adaptor<Bits, xstd::bits::detail::storage::borrowed>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_DETAIL_SET_ADAPTOR_HPP
