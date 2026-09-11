//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SET_ADAPTOR_HPP
#define XSTD_BITS_SET_ADAPTOR_HPP

#include <xstd/bits/bit_traits.hpp>           // bit_storage, bit_traits, count, find_first, find_next, find_prev, static_bit_extent
#include <xstd/bits/detail/bidirectional.hpp> // bidirectional_bit_iterator, bidirectional_bit_reference
#include <xstd/bits/detail/hash.hpp>          // hash_append_bits, hash_append_positions, std_hash
#include <xstd/bits/detail/intrin.hpp>        // countl_zero, countr_zero
#include <xstd/bits/ownership.hpp>            // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership, owns
#include <boost/container_hash/is_range.hpp>  // is_range
#include <boost/hash2/hash_append.hpp>        // hash_append_tag
#include <algorithm>                          // any_of, equal, includes, lexicographical_compare_three_way
#include <cassert>                            // assert
#include <compare>                            // strong_ordering
#include <concepts>                           // constructible_from, invocable, swappable
#include <cstddef>                            // ptrdiff_t, size_t
#include <functional>                         // hash, less
#include <initializer_list>                   // initializer_list
#include <limits>                             // numeric_limits
#include <iterator>                           // input_iterator, iter_reference_t, make_reverse_iterator, reverse_iterator, sentinel_for
#include <ranges>                             // begin, enable_borrowed_range, enable_view, end, input_range, range_reference_t, from_range_t, swap
#include <type_traits>                        // conditional_t, false_type, is_invocable_r_v, is_nothrow_swappable_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                            // forward, move, pair

// The set reading, [set] over any Bits with a bit_traits specialization, owning it or referring to it. [design.md#the-three-adaptors]
namespace xstd {

namespace detail::set {

// A range of consecutive ascending positions, which is what a block-wise fill needs and what a general input
// range cannot be asked. std::views::iota is the one that says so in its type. [design.md#the-range-members]
template<class R> inline constexpr bool is_consecutive = false;
template<class W, class B> inline constexpr bool is_consecutive<std::ranges::iota_view<W, B>> = true;

// Continue unless the functor says otherwise: a void functor always continues, a bool one says.
// [design.md#the-set-for-each]
//
// The position is handed over as a prvalue -- [expr.type.conv]'s decay-copy -- rather than as this parameter's name. A named lvalue binds to a
// functor taking std::size_t&, which then writes to a local that goes nowhere -- a walk reports positions and
// changes none, so the write is not merely lost but meaningless. A prvalue makes that a compile error, and it
// is what is_invocable_r_v just above already asks about, so the call and the detection stop disagreeing about
// the value category. [design.md#the-functor-takes-a-value]
template<class F>
[[nodiscard]] constexpr auto invoke_continues(F& f, std::size_t pos)
        -> bool
{
        if constexpr (std::is_invocable_r_v<bool, F&, std::size_t>) {
                return f(auto(pos));
        } else {
                f(auto(pos));
                return true;
        }
}

// One tier each, because the tier is the seam and sharing a body puts the whole over
// readability-function-cognitive-complexity's threshold. [design.md#one-function-per-tier]

// Blocks, lowest position first: load once per block, then tzcnt for the position and blsr to drop it.
template<class Traits, class Bits, class F>
constexpr auto walk_blocks_ascending(Bits const& c, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto index = 0UZ, blocks = Traits::num_blocks(c); index < blocks; ++index) {
                auto block = Traits::block(c, index);
                while (block != block_type{}) {
                        auto const offset = static_cast<std::size_t>(detail::bits::countr_zero(block));
                        if (not invoke_continues(f, (digits * index) + offset)) {
                                return;
                        }
                        block = static_cast<block_type>(block & static_cast<block_type>(block - block_type{1}));
                }
        }
}

// The mirror. w & (w - 1) has no descending twin, so this clears the bit it just reported.
template<class Traits, class Bits, class F>
constexpr auto walk_blocks_descending(Bits const& c, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto n = 0UZ, blocks = Traits::num_blocks(c); n < blocks; ++n) {
                auto const index = blocks - 1UZ - n;
                auto block = Traits::block(c, index);
                while (block != block_type{}) {
                        auto const offset = digits - 1UZ - static_cast<std::size_t>(detail::bits::countl_zero(block));
                        if (not invoke_continues(f, (digits * index) + offset)) {
                                return;
                        }
                        block = static_cast<block_type>(block ^ static_cast<block_type>(block_type{1} << offset));
                }
        }
}

// The other tier: a storage with no block access -- boost::dynamic_bitset is the one -- walks positions, which
// is what the iterator does and is still the same answer. [design.md#windows]
template<class Range, class F>
constexpr auto walk_positions_ascending(Range const& r, F& f)
        -> void
{
        for (auto const pos : r) {
                if (not invoke_continues(f, pos)) {
                        return;
                }
        }
}

template<class Range, class F>
constexpr auto walk_positions_descending(Range const& r, F& f)
        -> void
{
        for (auto it = r.rbegin(), last = r.rend(); it != last; ++it) {
                if (not invoke_continues(f, *it)) {
                        return;
                }
        }
}

}       // namespace detail::set


template<class Bits, ownership Own, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>>
class set_adaptor
{
        static constexpr bool is_owner = owns(Own);

        using bits_type = std::remove_const_t<Bits>;

        // Always present and only its type changes, so plain conditional_t. [design.md#ownership-is-not-an-axis]
        std::conditional_t<is_owner, Bits, Bits*> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow. [design.md#ownership-is-not-an-axis]
        [[nodiscard]] constexpr auto storage(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return self.m_bits;
                } else {
                        return *self.m_bits;
                }
        }

        // Either reading's view refers into this owner's storage, and nothing else outside does. [design.md#views-over-owners]
        template<class B, ownership O, bit_storage<B> T>         friend class set_adaptor;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class sequence_adaptor;

        // The value under the set reading, owned or viewed as == is: the bits at a static width, the positions at a run-time one, where two equal sets need not share a width. [design.md#the-hashing-invariant]
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, set_adaptor const* v) noexcept
                -> void
        {
                if constexpr (has_static_width) {
                        detail::bits::hash_append_bits<Traits>(h, f, v->storage());
                } else {
                        detail::bits::hash_append_positions<Traits>(h, f, v->storage());
                }
        }

public:
        // types
        using key_type               = std::size_t;
        using key_compare            = std::less<key_type>;
        using value_type             = key_type;
        using value_compare          = key_compare;
        using traits_type            = Traits;
        static constexpr bool has_static_width = static_bit_extent<Traits, Bits>;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = detail::bits::bidirectional_bit_reference<Bits, Traits>;
        using const_reference        = reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = detail::bits::bidirectional_bit_iterator<Bits, Traits>;
        using const_iterator         = iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy; an owner is built the way std::set is, a view only from what it views.
        [[nodiscard]] constexpr set_adaptor() noexcept requires is_owner = default;

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

        [[nodiscard]] constexpr explicit set_adaptor(Bits& c) noexcept
                requires (not is_owner)
        :
                m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. [design.md#views-over-owners]
        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit set_adaptor(Owner& c) noexcept
                requires (not is_owner)
        :
                m_bits(&c.m_bits)
        {}

        constexpr auto operator=(std::initializer_list<value_type> il)
                -> set_adaptor&
                requires is_owner
        {
                clear();
                insert(il.begin(), il.end());
                return *this;
        }

        // The storage's own equality, which every storage in the tree has; ordering is the trait's entry, or the invariant it must satisfy. [design.md#the-ordering-invariant]
        // Both over the elements when two run-time widths differ: the storages then cannot agree, the sets still can. [design.md#width-is-capacity]
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
                requires requires { { x.storage() == y.storage() } -> std::convertible_to<bool>; }
        {
                if (not same_width(x, y)) {
                        return std::ranges::equal(x, y);
                }
                return x.storage() == y.storage();
        }

        [[nodiscard]] friend constexpr auto operator<=>(set_adaptor const& x, set_adaptor const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (requires { Traits::set_three_way(x.storage(), y.storage()); }) {
                        if (same_width(x, y)) {
                                return Traits::set_three_way(x.storage(), y.storage());
                        }
                }
                return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
        }

        // iterators; one type for both, this reading being read-only through its proxy. [design.md#read-only-set-proxy]
        [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator { return { &storage(), detail::bits::find_first<Traits>(storage()) }; }
        [[nodiscard]] constexpr auto end()   const noexcept -> const_iterator { return { &storage(), Traits::size(storage()) }; }

        [[nodiscard]] constexpr auto rbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(end());   }
        [[nodiscard]] constexpr auto rend()   const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept -> const_iterator         { return begin();  }
        [[nodiscard]] constexpr auto cend()    const noexcept -> const_iterator         { return end();    }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return rbegin(); }
        [[nodiscard]] constexpr auto crend()   const noexcept -> const_reverse_iterator { return rend();   }

        // The set reading a block at a time, which is what an iterator cannot be. operator++ is flat: it must
        // re-derive the word from (pointer, position) on every step, because an iterator stays copyable and
        // restartable. A loop has somewhere to keep the block between positions, so it loads once per block and
        // spends two instructions per position -- tzcnt for the position, blsr to drop it. Measured 4.0x to 5.3x
        // over the range-for on every compiler tried, and the same on a two-word bitboard as at 2^22.
        // [design.md#the-set-for-each]
        //
        // The functor may return void, or bool to mean "keep going", which is what a move generator wants when it
        // has found its answer. Nothing else is offered: a functor that returns something else is a caller error
        // rather than a value to discard silently.
        //
        // It takes the position by value, and the constraint says so, so a functor asking for size_t& reads
        // "constraint not satisfied" here rather than compiling into a write that goes nowhere.
        // [design.md#the-functor-takes-a-value]
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                if constexpr (requires (std::size_t i) { Traits::block(self.storage(), i); Traits::num_blocks(self.storage()); }) {
                        detail::set::walk_blocks_ascending<Traits>(self.storage(), f);
                } else {
                        detail::set::walk_positions_ascending(self, f);
                }
        }

        // The mirror, highest position first. w & (w - 1) has no descending twin, so this one clears the top bit
        // it just reported instead. The set reading iterates both ways, and so does this. [design.md#the-set-for-each]
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each_reverse(this auto&& self, F f)
                -> void
        {
                if constexpr (requires (std::size_t i) { Traits::block(self.storage(), i); Traits::num_blocks(self.storage()); }) {
                        detail::set::walk_blocks_descending<Traits>(self.storage(), f);
                } else {
                        detail::set::walk_positions_descending(self, f);
                }
        }

        // capacity; a bitset's count() is a set's size(), and max_size() is the positions there are to hold. [design.md#max-size-is-the-bits]
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return begin() == end(); }
        [[nodiscard]] constexpr auto full()  const noexcept -> bool { return size() == max_size(); }

        [[nodiscard]] constexpr auto size() const noexcept -> size_type { return detail::bits::count<Traits>(storage()); }

        // [container.reqmts]/56, distance(begin(), end()) for the largest possible container: every position set, so the
        // width. A width in the type is that width, an owner grows to what its storage can address, and a view cannot
        // grow what it views, so it is that storage's width now. [design.md#max-size-is-the-bits]
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr (static_bit_extent<Traits, bits_type>) {
                        return Traits::extent;
                } else if constexpr (owns(Own)) {
                        return storage().max_size();
                } else {
                        return Traits::size(storage());
                }
        }

        // element access, both with a non-empty set as their precondition.
        [[nodiscard]] constexpr auto front() const noexcept -> const_reference { return *begin(); }
        [[nodiscard]] constexpr auto back()  const noexcept -> const_reference { return { &storage(), detail::bits::find_prev<Traits>(storage(), Traits::size(storage())) }; }

        // modifiers; each writes through Traits, so each exists exactly where Traits lets this handle write. [design.md#ownership-is-not-an-axis]
        template<class... Args>
        constexpr auto emplace(this auto&& self, Args&&... args)
                -> std::pair<iterator, bool>
                requires (sizeof...(args) == 1) and requires { Traits::insert(self.storage(), value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(this auto&& self, const_iterator position, Args&&... args)
                -> iterator
                requires (sizeof...(args) == 1) and requires { Traits::insert(self.storage(), value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(position, value_type(std::forward<Args>(args)...));
        }

        // [set]'s two overloads by value: a key is a size_t, and there is nothing to move.
        constexpr auto insert(this auto&& self, value_type x) -> std::pair<iterator, bool> requires requires { Traits::insert(self.storage(), x); } { return self.do_insert(x); }
        constexpr auto insert(this auto&& self, const_iterator position, value_type x) -> iterator requires requires { Traits::insert(self.storage(), x); } { return self.do_insert(position, x); }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr auto insert(this auto&& self, I first, S last)
                -> void
                requires std::constructible_from<value_type, std::iter_reference_t<I>> and requires { Traits::insert(self.storage(), static_cast<value_type>(*first)); }
        {
                for (; first != last; ++first) {
                        Traits::insert(self.storage(), static_cast<value_type>(*first));
                }
        }

        // Ranged insertion has tiers, as the sequence reading's append_range does. [design.md#the-range-members]
        template<std::ranges::input_range R>
        constexpr auto insert_range(this auto&& self, R&& rg)
                -> void
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>> and requires { Traits::insert(self.storage(), static_cast<value_type>(*std::ranges::begin(rg))); }
        {
                if constexpr (requires { self |= rg; }) {
                        // Tier one: another set over the same storage, which is a union and already knows how to do
                        // one block-wise, mismatched widths included.
                        self |= rg;
                } else if constexpr (detail::set::is_consecutive<std::remove_cvref_t<R>> and requires (std::size_t pos, std::size_t len) { self.storage().set(pos, len, true); }) {
                        // Tier two: consecutive positions, so the first and last blocks are masked and everything
                        // between them is written whole, which is what the ranged set does.
                        if (not std::ranges::empty(rg)) {
                                auto const lo  = static_cast<value_type>(*std::ranges::begin(rg));
                                auto const len = static_cast<std::size_t>(std::ranges::distance(rg));
                                // The last position first, so a growable storage is already wide enough for the fill
                                // and a fixed one asserts exactly where an element-wise insert would have.
                                Traits::insert(self.storage(), lo + len - 1UZ);
                                self.storage().set(lo, len, true);
                        }
                } else {
                        self.insert(std::ranges::begin(rg), std::ranges::end(rg));
                }
        }

        constexpr auto insert(this auto&& self, std::initializer_list<value_type> ilist)
                -> void
                requires requires { Traits::insert(self.storage(), *ilist.begin()); }
        {
                self.insert(ilist.begin(), ilist.end());
        }

        constexpr auto fill(this auto&& self) noexcept
                -> void
                requires requires { Traits::fill(self.storage(), true); }
        {
                Traits::fill(self.storage(), true);
        }

        // The successor first: exclusive_find_next never reads the position it steps from, but the order costs nothing and says so.
        constexpr auto erase(this auto&& self, const_iterator position) noexcept
                -> iterator
                requires requires { Traits::unchecked_assign(self.storage(), *position, false); }
        {
                assert(position != self.end());
                auto nrv = position;
                ++nrv;
                Traits::unchecked_assign(self.storage(), *position, false);
                return nrv;
        }

        // Total over key_type, as std::set's is: an absent key is the no-op returning zero. [design.md#total-lookups-on-the-container]
        constexpr auto erase(this auto&& self, key_type const& x) noexcept
                -> size_type
                requires requires { Traits::unchecked_assign(self.storage(), x, false); }
        {
                if (not self.contains(x)) {
                        return 0UZ;
                }
                Traits::unchecked_assign(self.storage(), x, false);
                return 1UZ;
        }

        constexpr auto erase(this auto&& self, const_iterator first, const_iterator last) noexcept
                -> iterator
                requires requires { Traits::unchecked_assign(self.storage(), *first, false); }
        {
                while (first != last) {
                        Traits::unchecked_assign(self.storage(), *first++, false);
                }
                return last;
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr auto swap(set_adaptor& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        // Asking the storage, as the other two adaptors do: a set over an allocating storage has one to show. [design.md#the-generated-table]
        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires is_owner and requires (Bits const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        constexpr auto clear(this auto&& self) noexcept
                -> void
                requires requires { Traits::fill(self.storage(), false); }
        {
                Traits::fill(self.storage(), false);
        }

        constexpr auto complement(this auto&& self, value_type x) noexcept
                -> void
                requires requires { Traits::unchecked_assign(self.storage(), x, true); }
        {
                assert(x < Traits::size(self.storage()));
                Traits::unchecked_assign(self.storage(), x, not Traits::at(self.storage(), x));
        }

        constexpr auto complement(this auto&& self) noexcept -> void requires requires { self.storage().flip(); } { self.storage().flip(); }

        // Bulk, on the storage's own spelling: what every storage agrees on is required of it, not reconciled. [design.md#what-the-trait-reconciles]
        // Two run-time widths that differ go element-wise instead, the storages' own being equal-width operations; the two that insert may then allocate. [design.md#width-is-capacity]
        constexpr auto operator&=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.storage() &= other.storage(); }
        {
                if (same_width(self, other)) {
                        self.storage() &= other.storage();
                } else {
                        for (auto const x : self) {
                                if (not other.contains(x)) {
                                        self.erase(x);
                                }
                        }
                }
                return self;
        }

        constexpr auto operator|=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage() |= other.storage(); }
        {
                if (same_width(self, other)) {
                        self.storage() |= other.storage();
                } else {
                        for (auto const x : other) {
                                self.insert(x);
                        }
                }
                return self;
        }

        constexpr auto operator^=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage() ^= other.storage(); }
        {
                if (same_width(self, other)) {
                        self.storage() ^= other.storage();
                } else {
                        for (auto const x : other) {
                                if (self.erase(x) == 0UZ) {
                                        self.insert(x);
                                }
                        }
                }
                return self;
        }

        constexpr auto operator-=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.storage() -= other.storage(); }
        {
                if (same_width(self, other)) {
                        self.storage() -= other.storage();
                } else {
                        for (auto const x : other) {
                                self.erase(x);
                        }
                }
                return self;
        }

        // The shifts translate the set. A run-time width grows to hold a left shift and empties past a right one, the width being no precondition. [design.md#width-is-capacity]
        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage() <<= n; } and (has_static_width or requires { self.storage().resize(n); })
        {
                if constexpr (has_static_width) {
                        self.storage() <<= n;
                } else if (auto const width = Traits::size(self.storage()); width > 0UZ) {
                        self.storage().resize(width + n);
                        self.storage() <<= n;
                }
                return self;
        }

        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept
                -> auto&
                requires requires { self.storage() >>= n; }
        {
                if constexpr (not has_static_width) {
                        if (n >= Traits::size(self.storage())) {
                                Traits::fill(self.storage(), false);
                                return self;
                        }
                }
                self.storage() >>= n;
                return self;
        }

        // observers
        [[nodiscard]] constexpr auto   key_comp() const noexcept -> key_compare   { return {}; }
        [[nodiscard]] constexpr auto value_comp() const noexcept -> value_compare { return {}; }

        // set operations, every one total over key_type as std::set's are; the width is the guard, Traits::at the read behind it. [design.md#total-lookups-on-the-container]
        [[nodiscard]] constexpr auto contains(key_type const& x) const noexcept -> bool      { return x < Traits::size(storage()) and Traits::at(storage(), x); }
        [[nodiscard]] constexpr auto count   (key_type const& x) const noexcept -> size_type { return contains(x); }

        [[nodiscard]] constexpr auto find(key_type const& x) const noexcept
                -> const_iterator
        {
                return contains(x) ? const_iterator{ &storage(), x } : end();
        }

        // The first element not less than x, asked about directly because stepping from x - 1 would underflow at zero.
        [[nodiscard]] constexpr auto lower_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                return contains(x) ? const_iterator{ &storage(), x } : upper_bound(x);
        }

        [[nodiscard]] constexpr auto upper_bound(key_type const& x) const noexcept
                -> const_iterator
        {
                if (x >= Traits::size(storage())) {
                        return end();
                }
                return { &storage(), detail::bits::find_next<Traits>(storage(), x) };
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) const noexcept
                -> std::pair<const_iterator, const_iterator>
        {
                return { lower_bound(x), upper_bound(x) };
        }

        // The storage's own member where it has one, its bulk operators otherwise: both block-wise, and every storage in the tree has one of the two.
        // Two run-time widths that differ are asked element-wise, as the comparisons are. [design.md#width-is-capacity]
        [[nodiscard]] constexpr auto is_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if (not same_width(*this, other)) {
                        return std::ranges::includes(other, *this);
                }
                if constexpr (requires { storage().is_subset_of(other.storage()); }) {
                        return storage().is_subset_of(other.storage());
                } else {
                        return (storage() & ~other.storage()).none();
                }
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if (not same_width(*this, other)) {
                        return is_subset_of(other) and size() != other.size();
                }
                if constexpr (requires { storage().is_proper_subset_of(other.storage()); }) {
                        return storage().is_proper_subset_of(other.storage());
                } else {
                        return is_subset_of(other) and *this != other;
                }
        }

        [[nodiscard]] constexpr auto intersects(set_adaptor const& other) const noexcept
                -> bool
        {
                if (not same_width(*this, other)) {
                        return std::ranges::any_of(*this, [&other](auto x) { return other.contains(x); });
                }
                if constexpr (requires { storage().intersects(other.storage()); }) {
                        return storage().intersects(other.storage());
                } else {
                        return (storage() & other.storage()).any();
                }
        }

private:
        // Constantly true at a static width, so every arm above folds to the storage's own. [design.md#width-is-capacity]
        [[nodiscard]] static constexpr auto same_width(set_adaptor const& x [[maybe_unused]], set_adaptor const& y [[maybe_unused]]) noexcept
                -> bool
        {
                if constexpr (has_static_width) {
                        return true;
                } else {
                        return Traits::size(x.storage()) == Traits::size(y.storage());
                }
        }

        // Traits::insert returns nothing, so "was it new" is asked first; total, an out-of-range key being the trait's precondition to refuse.
        constexpr auto do_insert(this auto&& self, value_type x)
                -> std::pair<iterator, bool>
        {
                auto const inserted = not self.contains(x);
                Traits::insert(self.storage(), x);
                return { { &self.storage(), x }, inserted };
        }

        constexpr auto do_insert(this auto&& self, const_iterator, value_type x)
                -> iterator
        {
                Traits::insert(self.storage(), x);
                return { &self.storage(), x };
        }
};

// A view deduces the constness of what it views, the way span<T> and span<T const> do; over an owner, of the storage it wraps.
// Constrained to non-owners: a bitset has a bit_traits of its own, so an unconstrained guide here is viable for an
// owner too and ties with the owner guide below, making bit_set_view(bs) ambiguous. The constraint used to sit on
// the view's restated guide; the view is an alias now and deduces through these. [design.md#a-bitset-reads-as-its-storage]
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
set_adaptor(Bits&) -> set_adaptor<Bits, ownership::refers>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
set_adaptor(Owner&) -> set_adaptor<owned_bits_t<Owner>, ownership::refers, owned_traits_t<Owner>>;

// The owner's side of the protocol above.
template<class Bits, class Traits>
struct owned_storage<set_adaptor<Bits, ownership::owns, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.
template<class Bits, ownership Own, class Traits>
constexpr auto swap(set_adaptor<Bits, Own, Traits>& x, set_adaptor<Bits, Own, Traits>& y) noexcept(noexcept(x.swap(y)))
        -> void
        requires (owns(Own))
{
        x.swap(y);
}

// 23.4.6.3 Erasure                                                [set.erasure]
template<class Bits, ownership Own, class Traits, class Predicate>
constexpr auto erase_if(set_adaptor<Bits, Own, Traits>& c, Predicate pred)
        -> set_adaptor<Bits, Own, Traits>::size_type
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

// The non-member forms copy, so they are the owner's alone: a copied view would write through to what it views; the copy may allocate at a run-time width.
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator~(set_adaptor<Bits, Own, Traits> const& lhs) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c.complement(); } { auto nrv = lhs; nrv.complement(); return nrv; }

template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator&(set_adaptor<Bits, Own, Traits> const& lhs, set_adaptor<Bits, Own, Traits> const& rhs) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c &= c; } { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator|(set_adaptor<Bits, Own, Traits> const& lhs, set_adaptor<Bits, Own, Traits> const& rhs) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c |= c; } { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator^(set_adaptor<Bits, Own, Traits> const& lhs, set_adaptor<Bits, Own, Traits> const& rhs) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c ^= c; } { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator-(set_adaptor<Bits, Own, Traits> const& lhs, set_adaptor<Bits, Own, Traits> const& rhs) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c -= c; } { auto nrv = lhs; nrv -= rhs; return nrv; }

template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator<<(set_adaptor<Bits, Own, Traits> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c <<= n; } { auto nrv = lhs; nrv <<= n; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator>>(set_adaptor<Bits, Own, Traits> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Own, Traits>::has_static_width) -> set_adaptor<Bits, Own, Traits> requires (owns(Own)) and requires (set_adaptor<Bits, Own, Traits> c) { c >>= n; } { auto nrv = lhs; nrv >>= n; return nrv; }
// NOLINTEND(readability-redundant-parentheses)

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright, so a pipeline takes it as it is; and borrowed, its iterators pointing at the storage and not at it. [design.md#views-follow-their-precedent]
template<class Bits, class Traits>
inline constexpr bool enable_view<xstd::set_adaptor<Bits, xstd::ownership::refers, Traits>> = true;

template<class Bits, class Traits>
inline constexpr bool enable_borrowed_range<xstd::set_adaptor<Bits, xstd::ownership::refers, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// Owned or viewed, as std::string_view hashes and std::set does not. [design.md#the-hashing-invariant]
template<class Bits, xstd::ownership Own, class Traits>
struct hash<xstd::set_adaptor<Bits, Own, Traits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::set_adaptor<Bits, Own, Traits> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

}       // namespace std
// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload, which cannot hash the proxy the set iterator returns. [design.md#the-hashing-invariant]
namespace boost::container_hash {

template<class Bits, xstd::ownership Own, class Traits>
struct is_range<xstd::set_adaptor<Bits, Own, Traits>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_SET_ADAPTOR_HPP
