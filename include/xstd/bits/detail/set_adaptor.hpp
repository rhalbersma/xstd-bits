//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_SET_ADAPTOR_HPP
#define XSTD_BITS_DETAIL_SET_ADAPTOR_HPP

#include <xstd/bits/bit_storage.hpp>                     // bit_storage
#include <xstd/bits/detail/allocator_base_type.hpp>      // allocator_base_type, allocator_param_t, has_allocator_v
#include <xstd/bits/detail/bidirectional.hpp>            // bidirectional_bit_iterator, bidirectional_bit_reference
#include <xstd/bits/detail/borrowed_bits.hpp>            // borrow_bits, borrowable_word, borrowable_words, borrowed_bits_t
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container, contiguous_bit_container_type
#include <xstd/bits/detail/functor.hpp>                  // decay_copy
#include <xstd/bits/detail/hash.hpp>                     // hash_append_bits, hash_append_positions, std_hash
#include <xstd/bits/detail/intrin.hpp>                   // countl_zero, countr_zero
#include <xstd/bits/detail/ownership.hpp>                // owned_bits_t, owned_storage, owner_of, owner_reading, storage, owns
#include <xstd/bits/detail/shift.hpp>                    // shl, shr
#include <xstd/bits/detail/storage_ptr.hpp>              // storage_ref_t
#include <xstd/bits/detail/zero_width.hpp>               // zero_width
#include <xstd/bits/from_bit_storage.hpp>                // from_bit_storage_t
#include <xstd/misc/type_traits/empty_base_type.hpp>     // empty_base_type
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/hash2/hash_append.hpp>                   // hash_append_tag
#include <algorithm>                                     // all_of, find_if, lexicographical_compare_three_way, max, min
#include <cassert>                                       // assert
#include <compare>                                       // strong_ordering
#include <concepts>                                      // constructible_from, convertible_to, invocable, same_as, swappable
#include <cstddef>                                       // ptrdiff_t, size_t
#include <format>                                        // format
#include <functional>                                    // hash, less
#include <initializer_list>                              // initializer_list
#include <iterator>                                      // input_iterator, iter_reference_t, make_reverse_iterator, reverse_iterator, sentinel_for
#include <ranges>                                        // begin, enable_borrowed_range, enable_view, end, input_range, iota, range_reference_t, from_range_t, swap, transform
#include <source_location>                               // source_location
#include <span>                                          // dynamic_extent
#include <stdexcept>                                     // out_of_range
#include <type_traits>                                   // conditional_t, false_type, is_invocable_r_v, is_nothrow_constructible_v, is_nothrow_default_constructible_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                                       // declval, forward, move, pair

// The set reading, [set] over a contiguous_bit_container, owning it or referring to it.
namespace xstd::bits::detail {

namespace set {

// The storage answering equality, named so the constrained overload below subsumes the general one.
template<class Bits>
concept equality_comparable_storage = requires (Bits const& a, Bits const& b) {
        { a == b } -> std::convertible_to<bool>;
};

// A range of consecutive ascending positions, which is what a block-wise fill needs.
template<class R>
inline constexpr bool is_consecutive = false;
template<class W, class B>
inline constexpr bool is_consecutive<std::ranges::iota_view<W, B>> = true;

// One tier each: sharing a body puts the whole over readability-function-cognitive-complexity.

// Blocks, lowest position first: load once per block, then tzcnt for the position and blsr to drop it.
template<class Bits, class F>
constexpr auto walk_blocks_ascending(Bits const& c, F& f)
        -> void
{
        using block_type = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;

        for (auto const index : std::views::iota(0UZ, c.num_blocks())) {
                auto block = c.block(index);
                while (block != block_type{}) {
                        auto const offset = static_cast<std::size_t>(countr_zero(block));
                        // A functor returning void has no exit to take, so its walk is compiled without one.
                        if constexpr (std::is_invocable_r_v<bool, F&, std::size_t>) {
                                if (not f(decay_copy((digits * index) + offset))) {
                                        return;
                                }
                        } else {
                                f(decay_copy((digits * index) + offset));
                        }
                        block = static_cast<block_type>(block & static_cast<block_type>(block - block_type{1}));
                }
        }
}

// The mirror. w & (w - 1) has no descending twin, so this clears the bit it just reported.
template<class Bits, class F>
constexpr auto walk_blocks_descending(Bits const& c, F& f)
        -> void
{
        using block_type = Bits::block_type;
        constexpr auto digits = Bits::bits_per_block;

        auto const blocks = c.num_blocks();
        for (auto index = blocks - 1UZ; index < blocks; --index) {
                auto block = c.block(index);
                while (block != block_type{}) {
                        auto const offset = digits - 1UZ - static_cast<std::size_t>(countl_zero(block));
                        // A functor returning void has no exit to take, so its walk is compiled without one.
                        if constexpr (std::is_invocable_r_v<bool, F&, std::size_t>) {
                                if (not f(decay_copy((digits * index) + offset))) {
                                        return;
                                }
                        } else {
                                f(decay_copy((digits * index) + offset));
                        }
                        block = static_cast<block_type>(block ^ shl(block_type{1}, offset));
                }
        }
}

} // namespace set

template<contiguous_bit_container_type Bits, storage Store = storage::owned, class Derived = void>
class set_adaptor : public std::conditional_t<owns(Store), allocator_base_type<std::remove_const_t<Bits>, set_adaptor<Bits, Store, Derived>>, xstd::empty_base_type<>>
{
        static constexpr bool is_owner = owns(Store);

        using bits_type = std::remove_const_t<Bits>;

        // Always present and only its type changes, so plain conditional_t.
        std::conditional_t<is_owner, Bits, storage_ref_t<Bits>> m_bits;

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

        // The container needs constraints only the vehicle can name; [class.friend]/3 ignores the void a view passes.
        friend Derived;

        // A view refers into this owner's storage, and only a reading that can view it is named.
        template<contiguous_bit_container_type, storage, class>
        friend class set_adaptor;

        // The value under the set reading: the bits at a static width, the positions at a run-time one.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, set_adaptor const* v) noexcept
                -> void
        {
                if constexpr (has_static_width) {
                        hash_append_bits(h, f, v->bits());
                } else {
                        hash_append_positions(h, f, v->bits());
                }
        }

public:
        // A vehicle used directly is its own container, which is what a view is.
        using derived_type = std::conditional_t<std::is_void_v<Derived>, set_adaptor, Derived>;

        // What a trait asks of this vehicle, every container built on it answering alike.
        using adaptor_type = set_adaptor;
        static constexpr auto reads_as = reading::set;
        using adapted_type = Bits;
        static constexpr bool owns_storage = is_owner;

        // types
        using key_type = std::size_t;
        using key_compare = std::less<key_type>;
        using value_type = key_type;
        using value_compare = key_compare;
        static constexpr bool has_static_width = (Bits::extent != std::dynamic_extent);
        using pointer = void;
        using const_pointer = pointer;
        using reference = bidirectional_bit_reference<Bits>;
        using const_reference = reference;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = bidirectional_bit_iterator<Bits>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
        // An allocator argument as [container.alloc.reqmts] takes it: converting, and only where the storage has one.
        static constexpr bool has_allocator = has_allocator_v<std::remove_const_t<Bits>>;
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

        // [set.cons]'s comparator arguments, taken and dropped: key_compare is std::less, which has no state to keep.
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
                requires is_owner and has_allocator
                : m_bits(alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(key_compare const& /* comp */, allocator_param const& alloc) noexcept(std::is_nothrow_constructible_v<Bits, allocator_param const&>)
                requires is_owner and has_allocator
                : m_bits(alloc)
        {}

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and has_allocator and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last, allocator_param const& alloc)
                : m_bits(alloc)
        {
                insert(first, last);
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and has_allocator and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr set_adaptor(I first, S last, key_compare const& /* comp */, allocator_param const& alloc)
                : set_adaptor(first, last, alloc)
        {}

        template<std::ranges::input_range R>
                requires is_owner and has_allocator and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg, allocator_param const& alloc)
                : set_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        template<std::ranges::input_range R>
                requires is_owner and has_allocator and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr set_adaptor(std::from_range_t, R&& rg, key_compare const& /* comp */, allocator_param const& alloc)
                : set_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il, allocator_param const& alloc)
                requires is_owner and has_allocator
                : set_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(std::initializer_list<value_type> il, key_compare const& /* comp */, allocator_param const& alloc)
                requires is_owner and has_allocator
                : set_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(set_adaptor const& other, allocator_param const& alloc)
                requires is_owner and has_allocator
                : m_bits(other.m_bits, alloc)
        {}

        [[nodiscard]] constexpr set_adaptor(set_adaptor&& other, allocator_param const& alloc)
                requires is_owner and has_allocator
                : m_bits(std::move(other.m_bits), alloc)
        {}

        // flat_set's adopting constructor at a run-time width: the blocks move in, every bit of them a position.
        [[nodiscard]] constexpr set_adaptor(xstd::from_bit_storage_t, bits_type::block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<typename bits_type::block_container_type>)
                requires is_owner and bits_type::has_stored_size
                : m_bits(xstd::from_bit_storage, std::move(blocks))
        {}

        [[nodiscard]] constexpr set_adaptor(xstd::from_bit_storage_t, bits_type::block_container_type blocks, allocator_param const& alloc)
                requires is_owner and bits_type::has_stored_size and has_allocator
                : m_bits(xstd::from_bit_storage, std::move(blocks), alloc)
        {}

        // Words that are bit storage, read as this set's positions; the tag says the words are bits and not keys.
        template<class B>
                requires is_owner and xstd::bit_storage<B> and Bits::template
        exchanges_bits<B> [[nodiscard]] constexpr set_adaptor(xstd::from_bit_storage_t, B const& b) noexcept
        {
                m_bits.assign_bits(b);
        }

        // Through bits() and not m_bits, which is a handle wherever this reading refers rather than owns.
        template<class B>
                requires Bits::template
        exchanges_bits<B> [[nodiscard]] constexpr auto to_bits() const noexcept
                -> B
        {
                return bits().template to_bits<B>();
        }

        [[nodiscard]] constexpr explicit set_adaptor(Bits& c) noexcept
                requires (not is_owner)
                : m_bits(&c)
        {}

        // Words handed straight over, held as the storage that borrows them, as std::views::all holds a view.
        template<class Words>
                requires (not is_owner) and (borrowable_word<Words &&> or borrowable_words<Words &&>) and std::same_as<borrowed_bits_t<Words&&>, Bits>
        [[nodiscard]] constexpr explicit set_adaptor(Words&& words) noexcept
                : m_bits(borrow_bits(std::forward<Words>(words)))
        {}

        // A view over an owner is a view over the storage it wraps; implicit, claiming nothing the owner lacks.
        template<owner_of<Bits, reading::set> Owner>
        [[nodiscard]] constexpr explicit(false) set_adaptor(Owner& c) noexcept // NOLINT(misc-explicit-constructor)
                requires (not is_owner)
                : m_bits(&c.m_bits)
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
                requires set::equality_comparable_storage<bits_type> and is_owner and has_static_width
        {
                return x.bits() == y.bits();
        }

        // Everything else: the storage's set equality, which answers at any two widths.
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
                requires set::equality_comparable_storage<bits_type>
        {
                return set_equal(x.bits(), y.bits());
        }

        // The storage's entry: the set ordering turns on the lowest position at which the two disagree.
        [[nodiscard]] friend constexpr auto operator<=>(set_adaptor const& x, set_adaptor const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (requires { set_lexicographical_compare_three_way(x.bits(), y.bits()); }) {
                        return set_lexicographical_compare_three_way(x.bits(), y.bits());
                } else {
                        return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                }
        }

        // iterators; one type for both, this reading being read-only through its proxy.
        [[nodiscard]] constexpr auto begin() const noexcept
                -> const_iterator
        {
                return {&bits(), bits().find_first()};
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

        // The set reading a block at a time, which is what an iterator cannot be.
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                set::walk_blocks_ascending(self.bits(), f);
        }

        // The mirror, highest position first.
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each_reverse(this auto&& self, F f)
                -> void
        {
                set::walk_blocks_descending(self.bits(), f);
        }

        // capacity; a bitset's count() is a set's size(), and max_size() is the positions there are to hold.
        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
        {
                return begin() == end();
        }

        [[nodiscard]] constexpr auto full() const noexcept
                -> bool
        {
                return size() == max_size();
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return bits().count();
        }

        // [container.reqmts]/56, distance(begin(), end()) for the largest container: every position set.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr ((bits_type::extent != std::dynamic_extent)) {
                        return bits_type::extent;
                } else if constexpr (owns(Store)) {
                        return bits().max_size();
                } else {
                        return bits().size();
                }
        }

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
                if constexpr (zero_width<bits_type>) {
                        return {&bits(), 0UZ};
                } else {
                        return {&bits(), bits().exclusive_find_prev(bits().size())};
                }
        }

        // modifiers; each writes through the storage, so each exists exactly where the storage lets this handle write.
        template<class... Args>
        constexpr auto emplace(this auto&& self, Args&&... args)
                -> std::pair<iterator, bool>
                requires (sizeof...(args) <= 1) and requires { self.bits().growing_insert(value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(this auto&& self, const_iterator position, Args&&... args)
                -> iterator
                requires (sizeof...(args) <= 1) and requires { self.bits().growing_insert(value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(position, value_type(std::forward<Args>(args)...));
        }

        // [set]'s two overloads by value: a key is a size_t, and there is nothing to move.
        constexpr auto insert(this auto&& self, value_type x) -> std::pair<iterator, bool>
                requires requires { self.bits().growing_insert(x); }
        {
                return self.do_insert(x);
        }

        constexpr auto insert(this auto&& self, const_iterator position, value_type x) -> iterator
                requires requires { self.bits().growing_insert(x); }
        {
                return self.do_insert(position, x);
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr auto insert(this auto&& self, I first, S last)
                -> void
                requires std::constructible_from<value_type, std::iter_reference_t<I>> and requires { self.bits().growing_insert(static_cast<value_type>(*first)); }
        {
                for (; first != last; ++first) {
                        auto const x = static_cast<value_type>(*first);
                        self.guard_key(x);
                        self.bits().growing_insert(x);
                }
        }

        // Ranged insertion has tiers, as the sequence reading's append_range does.
        template<std::ranges::input_range R>
        constexpr auto insert_range(this auto&& self, R&& rg)
                -> void
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>> and requires { self.bits().growing_insert(static_cast<value_type>(*std::ranges::begin(rg))); }
        {
                if constexpr (requires { self |= rg; }) {
                        // Tier one: another set over the same storage, which is a union done block-wise.
                        self |= rg;
                } else if constexpr (set::is_consecutive<std::remove_cvref_t<R>> and requires (std::size_t pos, std::size_t len) { self.bits().set(pos, len, true); }) {
                        // Tier two: consecutive positions, the first and last blocks masked, the rest whole.
                        if (not std::ranges::empty(rg)) {
                                auto const lo = static_cast<value_type>(*std::ranges::begin(rg));
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
                requires requires { self.bits().growing_insert(*ilist.begin()); }
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
                requires requires { self.bits().assign(*position, false); }
        {
                assert(position != self.end());
                auto nrv = position;
                ++nrv;
                self.bits().assign(*position, false);
                return nrv;
        }

        // Total over key_type, as std::set's is: an absent key is the no-op returning zero.
        constexpr auto erase(this auto&& self, key_type const& x) noexcept
                -> size_type
                requires requires { self.bits().assign(x, false); }
        {
                if (not self.contains(x)) {
                        return 0UZ;
                }
                self.bits().assign(x, false);
                return 1UZ;
        }

        constexpr auto erase(this auto&& self, const_iterator first, const_iterator last) noexcept
                -> iterator
                requires requires { self.bits().assign(*first, false); }
        {
                // A range, not two positions: reversed, the walk steps past last into a scan nothing answers.
                assert(static_cast<key_type>(*first) <= static_cast<key_type>(*last));
                while (first != last) {
                        self.bits().assign(*first++, false);
                }
                return last;
        }

        // The non-member beside it: ranges::swap finds this and never the member.
        friend constexpr auto swap(set_adaptor& x, set_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
                requires is_owner
        {
                x.swap(y);
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr auto swap(set_adaptor& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        // Asking the storage, as the other two adaptors do: a set over an allocating storage has one to show.
        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires is_owner and requires (Bits const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        // flat_set's door onto its representation, at a run-time width: the blocks come in and go out whole.
        using block_container_type = Bits::block_container_type;

        constexpr auto replace(block_container_type&& blocks) noexcept(noexcept(m_bits.replace(std::move(blocks))))
                -> void
                requires is_owner and requires (Bits& b, block_container_type&& c) { b.replace(std::move(c)); }
        {
                m_bits.replace(std::move(blocks));
        }

        [[nodiscard]] constexpr auto extract() && noexcept(noexcept(std::move(m_bits).extract()))
                -> block_container_type
                requires is_owner and requires (Bits&& b) { std::move(b).extract(); }
        {
                return std::move(m_bits).extract();
        }

        constexpr auto clear(this auto&& self) noexcept
                -> void
                requires requires { self.bits().fill(false); }
        {
                self.bits().fill(false);
        }

        // Toggling one key, growing where insert grows; not noexcept, since growing allocates.
        constexpr auto complement(this auto&& self, value_type x)
                -> void
                requires requires { self.bits().assign(x, true); }
        {
                self.guard_key(x);
                if constexpr (not has_static_width and requires { self.bits().growing_insert(x); }) {
                        if (x >= self.bits().size()) {
                                static_cast<void>(self.bits().growing_insert(x));
                                return;
                        }
                }
                assert(x < self.bits().size());
                self.bits().assign(x, not self.bits().test(x));
        }

        // The whole-set complement, at a static width alone: complementing needs a universe, which N is.
        constexpr auto complement(this auto&& self) noexcept -> void
                requires has_static_width and requires { self.bits().flip(); }
        {
                self.bits().flip();
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

        // The shifts translate the set; a run-time width grows for a left shift and empties past a right.
        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept(has_static_width)
                -> auto&
                requires requires { self.bits() <<= n; } and (has_static_width or requires { self.bits().resize(n); })
        {
                if constexpr (has_static_width) {
                        self.bits() <<= n;
                } else if (auto const width = self.bits().size(); width > 0UZ) {
                        // width + n through the saturating sum; past the positions there are it is length_error.
                        self.bits().resize(bits_type::check_width(bits_type::width_sum(width, n)));
                        self.bits() <<= n;
                }
                return self;
        }

        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept
                -> auto&
                requires requires { self.bits() >>= n; }
        {
                if constexpr (not has_static_width) {
                        if (n >= self.bits().size()) {
                                self.bits().fill(false);
                                return self;
                        }
                }
                self.bits() >>= n;
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

        // set operations, total over key_type as std::set's are; the width is the guard, test() the read.
        [[nodiscard]] constexpr auto contains(key_type const& x) const noexcept
                -> bool
        {
                return x < bits().size() and bits().test(x);
        }

        [[nodiscard]] constexpr auto count(key_type const& x) const noexcept
                -> size_type
        {
                return contains(x);
        }

        [[nodiscard]] constexpr auto find(key_type const& x) const noexcept
                -> const_iterator
        {
                return contains(x) ? const_iterator{&bits(), x} : end();
        }

        // The first element not less than x, asked about directly because stepping from x - 1 would underflow at zero.
        [[nodiscard]] constexpr auto lower_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                return contains(x) ? const_iterator{&bits(), x} : upper_bound(x);
        }

        [[nodiscard]] constexpr auto upper_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                if (x >= bits().size()) {
                        return end();
                }
                return {&bits(), bits().exclusive_find_next(x)};
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) const noexcept
                -> std::pair<const_iterator, const_iterator>
        {
                return {lower_bound(x), upper_bound(x)};
        }

        // The storage's own member where it has one, its bulk operators otherwise.
        [[nodiscard]] constexpr auto is_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if constexpr (requires { bits().is_subset_of(other.bits()); }) {
                        return bits().is_subset_of(other.bits());
                } else {
                        return (bits() & ~other.bits()).none();
                }
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if constexpr (requires { bits().is_proper_subset_of(other.bits()); }) {
                        return bits().is_proper_subset_of(other.bits());
                } else {
                        return is_subset_of(other) and *this != other;
                }
        }

        // A hidden friend: intersects is to set_intersection what contains is to find.
        [[nodiscard]] friend constexpr auto intersects(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
        {
                if constexpr (requires { intersects(x.bits(), y.bits()); }) {
                        return intersects(x.bits(), y.bits());
                } else {
                        return (x.bits() & y.bits()).any();
                }
        }

private:
        // The container built on this vehicle, which is what its value-returning operators hand back.
        [[nodiscard]] constexpr auto self() noexcept
                -> derived_type&
        {
                return static_cast<derived_type&>(*this);
        }

        // The one key a set of this reading can be unable to hold; every other member is total over key_type.
        constexpr auto guard_key(std::size_t x) const
                -> void
        {
                if constexpr (has_static_width) {
                        if (x >= max_size()) {
                                throw out_of_range(x);
                        }
                } else {
                        // A dynamic width refuses only what it could never grow to, and says length_error.
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
        constexpr auto do_insert(this auto&& self, value_type x)
                -> std::pair<iterator, bool>
        {
                self.guard_key(x);
                auto const inserted = self.bits().growing_insert(x);
                return {{&self.bits(), x}, inserted};
        }

        constexpr auto do_insert(this auto&& self, const_iterator, value_type x)
                -> iterator
        {
                self.guard_key(x);
                self.bits().growing_insert(x);
                return {&self.bits(), x};
        }
};

// Any container built on the set vehicle, the vehicle used directly included.
template<class T>
concept set_adaptor_like = requires { typename T::adaptor_type; T::reads_as; } and T::reads_as == reading::set and std::derived_from<T, typename T::adaptor_type>;

// The owner's side of the protocol above.
template<class Bits, class Derived>
struct owned_storage<set_adaptor<Bits, storage::owned, Derived>>
{
        using bits_type = Bits;

        // Committed to the set reading, so only a set view refers into one.
        static constexpr auto reads = reading::set;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the clause needs them.

// 23.4.6.3 Erasure                                                [set.erasure]
template<class Bits, storage Store, class Derived, class Predicate>
constexpr auto erase_if(set_adaptor<Bits, Store, Derived>& c, Predicate pred)
        -> set_adaptor<Bits, Store, Derived>::size_type
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
template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator~(set_adaptor<Bits, Store, Derived> const& lhs) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c.complement(); }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv.complement();
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator&(set_adaptor<Bits, Store, Derived> const& lhs, set_adaptor<Bits, Store, Derived> const& rhs) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c &= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv &= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator|(set_adaptor<Bits, Store, Derived> const& lhs, set_adaptor<Bits, Store, Derived> const& rhs) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c |= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv |= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator^(set_adaptor<Bits, Store, Derived> const& lhs, set_adaptor<Bits, Store, Derived> const& rhs) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c ^= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv ^= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator-(set_adaptor<Bits, Store, Derived> const& lhs, set_adaptor<Bits, Store, Derived> const& rhs) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c -= c; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv -= rhs;
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator<<(set_adaptor<Bits, Store, Derived> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c <<= n; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv <<= n;
        return nrv;
}

template<class Bits, storage Store, class Derived>
[[nodiscard]] constexpr auto operator>>(set_adaptor<Bits, Store, Derived> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Store, Derived>::has_static_width) -> set_adaptor<Bits, Store, Derived>::derived_type
        requires (owns(Store)) and requires (set_adaptor<Bits, Store, Derived> c) { c >>= n; }
{
        auto nrv = static_cast<set_adaptor<Bits, Store, Derived>::derived_type const&>(lhs);
        nrv >>= n;
        return nrv;
}

// NOLINTEND(readability-redundant-parentheses)

} // namespace xstd::bits::detail

namespace boost::container_hash {

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload.
template<class Bits, xstd::bits::detail::storage Store, class Derived>
struct is_range<xstd::bits::detail::set_adaptor<Bits, Store, Derived>> : std::false_type
{};

} // namespace boost::container_hash

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// Owned or viewed, as std::string_view hashes and std::set does not.
template<class Bits, xstd::bits::detail::storage Store, class Derived>
struct hash<xstd::bits::detail::set_adaptor<Bits, Store, Derived>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bits::detail::set_adaptor<Bits, Store, Derived> const& v) const noexcept
                -> std::size_t
        {
                return xstd::bits::detail::std_hash(v);
        }
};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

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
