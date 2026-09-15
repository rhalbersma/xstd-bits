//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SET_ADAPTOR_HPP
#define XSTD_BITS_SET_ADAPTOR_HPP

#include <xstd/bits/detail/bidirectional.hpp>            // bidirectional_bit_iterator, bidirectional_bit_reference
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/hash.hpp>                     // hash_append_bits, hash_append_positions, std_hash
#include <xstd/bits/detail/intrin.hpp>                   // countl_zero, countr_zero
#include <xstd/bits/detail/shift.hpp>                    // shl, shr
#include <xstd/bits/detail/zero_width.hpp>               // zero_width
#include <xstd/bits/ownership.hpp>                       // owned_bits_t, owned_storage, owner_of, owner_reading, ownership, owns, reading
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/hash2/hash_append.hpp>                   // hash_append_tag
#include <algorithm>                                     // all_of, find_if, lexicographical_compare_three_way, max, min
#include <cassert>                                       // assert
#include <compare>                                       // strong_ordering
#include <concepts>                                      // constructible_from, convertible_to, invocable, swappable
#include <cstddef>                                       // ptrdiff_t, size_t
#include <functional>                                    // hash, less
#include <initializer_list>                              // initializer_list
#include <iterator>                                      // input_iterator, iter_reference_t, make_reverse_iterator, reverse_iterator, sentinel_for
#include <limits>                                        // numeric_limits
#include <ranges>                                        // begin, enable_borrowed_range, enable_view, end, input_range, iota, range_reference_t, from_range_t, swap, transform
#include <span>                                          // dynamic_extent
#include <type_traits>                                   // conditional_t, false_type, is_invocable_r_v, is_nothrow_swappable_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                                       // declval, forward, move, pair

// The set reading, [set] over a contiguous_bit_container, owning it or referring to it.
namespace xstd {

namespace detail::set {

// The storage answering equality, named rather than spelled twice: two appearances of one requires-expression are distinct atomic constraints, so only a concept-id lets the constrained overload below subsume the general one.
template<class Bits>
concept equality_comparable_storage = requires (Bits const& a, Bits const& b) {
        { a == b } -> std::convertible_to<bool>;
};

// A range of consecutive ascending positions, which is what a block-wise fill needs and what a general input range cannot be asked.
template<class R> inline constexpr bool is_consecutive = false;
template<class W, class B> inline constexpr bool is_consecutive<std::ranges::iota_view<W, B>> = true;

// Continue unless the functor says otherwise: a void functor always continues, a bool one says.
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

// One tier each, because the tier is the seam and sharing a body puts the whole over readability-function-cognitive-complexity's threshold.

// Blocks, lowest position first: load once per block, then tzcnt for the position and blsr to drop it.
template<class Bits, class F>
constexpr auto walk_blocks_ascending(Bits const& c, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto index = 0UZ, blocks = c.num_blocks(); index < blocks; ++index) {
                auto block = c.block(index);
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
template<class Bits, class F>
constexpr auto walk_blocks_descending(Bits const& c, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto n = 0UZ, blocks = c.num_blocks(); n < blocks; ++n) {
                auto const index = blocks - 1UZ - n;
                auto block = c.block(index);
                while (block != block_type{}) {
                        auto const offset = digits - 1UZ - static_cast<std::size_t>(detail::bits::countl_zero(block));
                        if (not invoke_continues(f, (digits * index) + offset)) {
                                return;
                        }
                        block = static_cast<block_type>(block ^ detail::bits::shl(block_type{1}, offset));
                }
        }
}

}       // namespace detail::set


template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, ownership Own>
class set_adaptor
{
        static constexpr bool is_owner = owns(Own);

        using bits_type = std::remove_const_t<Bits>;

        // Always present and only its type changes, so plain conditional_t.
        std::conditional_t<is_owner, Bits, Bits*> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow.
        [[nodiscard]] constexpr auto storage(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return self.m_bits;
                } else {
                        return *self.m_bits;
                }
        }

        // A set view refers into this owner's storage, and nothing else outside does; a sequence view does not, the readings not mixing.
        template<specialization_of_TN<detail::bits::contiguous_bit_container> B, ownership O> friend class set_adaptor;

        // The value under the set reading, owned or viewed as == is: the bits at a static width, the positions at a run-time one, where two equal sets need not share a width.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, set_adaptor const* v) noexcept
                -> void
        {
                if constexpr (has_static_width) {
                        detail::bits::hash_append_bits(h, f, v->storage());
                } else {
                        detail::bits::hash_append_positions(h, f, v->storage());
                }
        }

public:
        // types
        using key_type               = std::size_t;
        using key_compare            = std::less<key_type>;
        using value_type             = key_type;
        using value_compare          = key_compare;
        static constexpr bool has_static_width = (Bits::extent != std::dynamic_extent);
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = detail::bits::bidirectional_bit_reference<Bits>;
        using const_reference        = reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = detail::bits::bidirectional_bit_iterator<Bits>;
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

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. Implicit, unlike the one above: it asserts nothing the owner does not already carry, which is the line span draws.
        template<owner_of<Bits, reading::set> Owner>
        [[nodiscard]] constexpr explicit(false) set_adaptor(Owner& c) noexcept  // NOLINT(misc-explicit-constructor)
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

        // A static owner's equality IS its one member's: every instance carries the same width, so the arms below have nothing to choose between. Said here rather than left to fold, the fact being about the type and not about the optimizer. The conjunction is what picks this one -- it subsumes the general overload's lone clause, so no negation is needed there.
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const&, set_adaptor const&) noexcept
                -> bool
                requires detail::set::equality_comparable_storage<bits_type> and is_owner and has_static_width = default;

        // Everything else: the storage's set equality, which answers at any two widths. Width is capacity for this reading, so two storages holding the same positions are equal whatever their widths, and a view holds a pointer that a defaulted comparison would compare in place of the contents.
        [[nodiscard]] friend constexpr auto operator==(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
                requires detail::set::equality_comparable_storage<bits_type>
        {
                return set_equal(x.storage(), y.storage());
        }

        // The storage's entry, which answers at any two widths: the set ordering turns on the lowest position at which the two disagree, and finding it is block work the storage is the place for.
        [[nodiscard]] friend constexpr auto operator<=>(set_adaptor const& x, set_adaptor const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (requires { set_lexicographical_compare_three_way(x.storage(), y.storage()); }) {
                        return set_lexicographical_compare_three_way(x.storage(), y.storage());
                } else {
                        return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                }
        }

        // iterators; one type for both, this reading being read-only through its proxy.
        [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator { return { &storage(), storage().find_first() }; }
        [[nodiscard]] constexpr auto end()   const noexcept -> const_iterator { return { &storage(), storage().size() }; }

        [[nodiscard]] constexpr auto rbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(end());   }
        [[nodiscard]] constexpr auto rend()   const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept -> const_iterator         { return begin();  }
        [[nodiscard]] constexpr auto cend()    const noexcept -> const_iterator         { return end();    }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return rbegin(); }
        [[nodiscard]] constexpr auto crend()   const noexcept -> const_reverse_iterator { return rend();   }

        // The set reading a block at a time, which is what an iterator cannot be.
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                detail::set::walk_blocks_ascending(self.storage(), f);
        }

        // The mirror, highest position first.
        template<class F>
                requires std::invocable<F&, std::size_t>
        constexpr auto for_each_reverse(this auto&& self, F f)
                -> void
        {
                detail::set::walk_blocks_descending(self.storage(), f);
        }

        // capacity; a bitset's count() is a set's size(), and max_size() is the positions there are to hold.
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return begin() == end(); }
        [[nodiscard]] constexpr auto full()  const noexcept -> bool { return size() == max_size(); }

        [[nodiscard]] constexpr auto size() const noexcept -> size_type { return storage().count(); }

        // [container.reqmts]/56, distance(begin(), end()) for the largest possible container: every position set, so the width.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr ((bits_type::extent != std::dynamic_extent)) {
                        return bits_type::extent;
                } else if constexpr (owns(Own)) {
                        return storage().max_size();
                } else {
                        return storage().size();
                }
        }

        // element access, both with a non-empty set as their precondition.
        [[nodiscard]] constexpr auto front() const noexcept -> const_reference { return *begin(); }
        // A zero width has no position to scan back from and exclusive_find_prev asserts there, where the trait's scan answered 0 without reaching the storage. back() on an empty set is a precondition violation either way, but the answer at a zero width stays what it was.
        [[nodiscard]] constexpr auto back() const noexcept
                -> const_reference
        {
                if constexpr (detail::bits::zero_width<bits_type>) {
                        return { &storage(), 0UZ };
                } else {
                        return { &storage(), storage().exclusive_find_prev(storage().size()) };
                }
        }

        // modifiers; each writes through the storage, so each exists exactly where the storage lets this handle write.
        template<class... Args>
        constexpr auto emplace(this auto&& self, Args&&... args)
                -> std::pair<iterator, bool>
                requires (sizeof...(args) == 1) and requires { self.storage().growing_insert(value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(this auto&& self, const_iterator position, Args&&... args)
                -> iterator
                requires (sizeof...(args) == 1) and requires { self.storage().growing_insert(value_type(std::forward<Args>(args)...)); }
        {
                return self.do_insert(position, value_type(std::forward<Args>(args)...));
        }

        // [set]'s two overloads by value: a key is a size_t, and there is nothing to move.
        constexpr auto insert(this auto&& self, value_type x) -> std::pair<iterator, bool> requires requires { self.storage().growing_insert(x); } { return self.do_insert(x); }
        constexpr auto insert(this auto&& self, const_iterator position, value_type x) -> iterator requires requires { self.storage().growing_insert(x); } { return self.do_insert(position, x); }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr auto insert(this auto&& self, I first, S last)
                -> void
                requires std::constructible_from<value_type, std::iter_reference_t<I>> and requires { self.storage().growing_insert(static_cast<value_type>(*first)); }
        {
                for (; first != last; ++first) {
                        self.storage().growing_insert(static_cast<value_type>(*first));
                }
        }

        // Ranged insertion has tiers, as the sequence reading's append_range does.
        template<std::ranges::input_range R>
        constexpr auto insert_range(this auto&& self, R&& rg)
                -> void
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>> and requires { self.storage().growing_insert(static_cast<value_type>(*std::ranges::begin(rg))); }
        {
                if constexpr (requires { self |= rg; }) {
                        // Tier one: another set over the same storage, which is a union and already knows how to do one block-wise, mismatched widths included.
                        self |= rg;
                } else if constexpr (detail::set::is_consecutive<std::remove_cvref_t<R>> and requires (std::size_t pos, std::size_t len) { self.storage().set(pos, len, true); }) {
                        // Tier two: consecutive positions, so the first and last blocks are masked and everything between them is written whole, which is what the ranged set does.
                        if (not std::ranges::empty(rg)) {
                                auto const lo  = static_cast<value_type>(*std::ranges::begin(rg));
                                auto const len = static_cast<std::size_t>(std::ranges::distance(rg));
                                // The last position first, so a growable storage is already wide enough for the fill and a fixed one asserts exactly where an element-wise insert would have.
                                self.storage().growing_insert(lo + len - 1UZ);
                                self.storage().set(lo, len, true);
                        }
                } else {
                        self.insert(std::ranges::begin(rg), std::ranges::end(rg));
                }
        }

        constexpr auto insert(this auto&& self, std::initializer_list<value_type> ilist)
                -> void
                requires requires { self.storage().growing_insert(*ilist.begin()); }
        {
                self.insert(ilist.begin(), ilist.end());
        }

        constexpr auto fill(this auto&& self) noexcept
                -> void
                requires requires { self.storage().fill( true); }
        {
                self.storage().fill( true);
        }

        // The successor first: exclusive_find_next never reads the position it steps from, but the order costs nothing and says so.
        constexpr auto erase(this auto&& self, const_iterator position) noexcept
                -> iterator
                requires requires { self.storage().assign( *position, false); }
        {
                assert(position != self.end());
                auto nrv = position;
                ++nrv;
                self.storage().assign( *position, false);
                return nrv;
        }

        // Total over key_type, as std::set's is: an absent key is the no-op returning zero.
        constexpr auto erase(this auto&& self, key_type const& x) noexcept
                -> size_type
                requires requires { self.storage().assign( x, false); }
        {
                if (not self.contains(x)) {
                        return 0UZ;
                }
                self.storage().assign( x, false);
                return 1UZ;
        }

        constexpr auto erase(this auto&& self, const_iterator first, const_iterator last) noexcept
                -> iterator
                requires requires { self.storage().assign( *first, false); }
        {
                while (first != last) {
                        self.storage().assign( *first++, false);
                }
                return last;
        }

        // The non-member beside it, hidden though the operators here are namespace-scope templates: ranges::swap finds this and never the member, and xstd::swap(a, b) is a spelling people reach for by habit where a qualified operator is not.
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

        constexpr auto clear(this auto&& self) noexcept
                -> void
                requires requires { self.storage().fill( false); }
        {
                self.storage().fill( false);
        }

        constexpr auto complement(this auto&& self, value_type x) noexcept
                -> void
                requires requires { self.storage().assign( x, true); }
        {
                assert(x < self.storage().size());
                self.storage().assign( x, not self.storage().test(x));
        }

        // The whole-set complement, at a static width alone: complementing needs a universe, and a run-time width is capacity rather than one ([width-is-capacity]). At a static width N is the universe and part of the type, so equal sets complement alike; at a run-time width two equal sets of different capacity would not, which is the width read as value that this reading does not do.
        constexpr auto complement(this auto&& self) noexcept -> void requires has_static_width and requires { self.storage().flip(); } { self.storage().flip(); }

        // Bulk, on the storage's own spelling, which is total across two widths. The set reading adds one thing the storage's operator deliberately does not: union and symmetric difference GROW, because for a set the width is capacity and an element the other holds above this width is still an element. Intersection and difference never widen, so they are the operator alone.
        constexpr auto operator&=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.storage() &= other.storage(); }
        {
                self.storage() &= other.storage();
                return self;
        }

        constexpr auto operator|=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage().grow_to_admit(other.storage()); self.storage() |= other.storage(); }
        {
                self.storage().grow_to_admit(other.storage());
                self.storage() |= other.storage();
                return self;
        }

        constexpr auto operator^=(this auto&& self, set_adaptor const& other) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage().grow_to_admit(other.storage()); self.storage() ^= other.storage(); }
        {
                self.storage().grow_to_admit(other.storage());
                self.storage() ^= other.storage();
                return self;
        }

        constexpr auto operator-=(this auto&& self, set_adaptor const& other) noexcept
                -> auto&
                requires requires { self.storage() -= other.storage(); }
        {
                self.storage() -= other.storage();
                return self;
        }

        // The shifts translate the set. A run-time width grows to hold a left shift and empties past a right one, the width being no precondition.
        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept(has_static_width)
                -> auto&
                requires requires { self.storage() <<= n; } and (has_static_width or requires { self.storage().resize(n); })
        {
                if constexpr (has_static_width) {
                        self.storage() <<= n;
                } else if (auto const width = self.storage().size(); width > 0UZ) {
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
                        if (n >= self.storage().size()) {
                                self.storage().fill( false);
                                return self;
                        }
                }
                self.storage() >>= n;
                return self;
        }

        // observers
        [[nodiscard]] constexpr auto   key_comp() const noexcept -> key_compare   { return {}; }
        [[nodiscard]] constexpr auto value_comp() const noexcept -> value_compare { return {}; }

        // set operations, every one total over key_type as std::set's are; the width is the guard, test() the read behind it.
        [[nodiscard]] constexpr auto contains(key_type const& x) const noexcept -> bool      { return x < storage().size() and storage().test(x); }
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
                if (x >= storage().size()) {
                        return end();
                }
                return { &storage(), storage().exclusive_find_next(x) };
        }

        [[nodiscard]] constexpr auto equal_range(key_type const& x) const noexcept
                -> std::pair<const_iterator, const_iterator>
        {
                return { lower_bound(x), upper_bound(x) };
        }

        // The storage's own member where it has one, its bulk operators otherwise. Every entry the storage offers answers at any two widths, so these are calls and not decisions.
        [[nodiscard]] constexpr auto is_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if constexpr (requires { storage().is_subset_of(other.storage()); }) {
                        return storage().is_subset_of(other.storage());
                } else {
                        return (storage() & ~other.storage()).none();
                }
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(set_adaptor const& other) const noexcept
                -> bool
        {
                if constexpr (requires { storage().is_proper_subset_of(other.storage()); }) {
                        return storage().is_proper_subset_of(other.storage());
                } else {
                        return is_subset_of(other) and *this != other;
                }
        }

        // A hidden friend where the bitset reading keeps a member: intersects is to set_intersection what contains is to find, and set_intersection is a free algorithm over two ranges where find is a member over one set and a key. std::set has no counterpart to mimic here, so nothing asks for the member the way boost asks it of bitset_adaptor -- and having none is also what lets this reach the storage's own friend, which a member of the same name would hide.
        [[nodiscard]] friend constexpr auto intersects(set_adaptor const& x, set_adaptor const& y) noexcept
                -> bool
        {
                if constexpr (requires { intersects(x.storage(), y.storage()); }) {
                        return intersects(x.storage(), y.storage());
                } else {
                        return (x.storage() & y.storage()).any();
                }
        }

private:
        // growing_insert reports whether the bit was new, so the contains() pass that asked it first is gone: one walk where there were two, and the same answer, an out-of-range key growing the storage to admit it.
        constexpr auto do_insert(this auto&& self, value_type x)
                -> std::pair<iterator, bool>
        {
                auto const inserted = self.storage().growing_insert(x);
                return { { &self.storage(), x }, inserted };
        }

        constexpr auto do_insert(this auto&& self, const_iterator, value_type x)
                -> iterator
        {
                self.storage().growing_insert(x);
                return { &self.storage(), x };
        }
};

// A view deduces the constness of what it views, the way span<T> and span<T const> do; over an owner, of the storage it wraps.
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
set_adaptor(Bits&) -> set_adaptor<Bits, ownership::refers>;

template<owner_reading<reading::set> Owner>
set_adaptor(Owner&) -> set_adaptor<owned_bits_t<Owner>, ownership::refers>;

// The owner's side of the protocol above.
template<class Bits>
struct owned_storage<set_adaptor<Bits, ownership::owns>>
{
        using bits_type   = Bits;

        // Committed to the set reading, so only a set view refers into one.
        static constexpr auto reads = reading::set;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.

// 23.4.6.3 Erasure                                                [set.erasure]
template<class Bits, ownership Own, class Predicate>
constexpr auto erase_if(set_adaptor<Bits, Own>& c, Predicate pred)
        -> set_adaptor<Bits, Own>::size_type
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
template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator~(set_adaptor<Bits, Own> const& lhs) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c.complement(); } { auto nrv = lhs; nrv.complement(); return nrv; }

template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator&(set_adaptor<Bits, Own> const& lhs, set_adaptor<Bits, Own> const& rhs) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c &= c; } { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator|(set_adaptor<Bits, Own> const& lhs, set_adaptor<Bits, Own> const& rhs) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c |= c; } { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator^(set_adaptor<Bits, Own> const& lhs, set_adaptor<Bits, Own> const& rhs) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c ^= c; } { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator-(set_adaptor<Bits, Own> const& lhs, set_adaptor<Bits, Own> const& rhs) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c -= c; } { auto nrv = lhs; nrv -= rhs; return nrv; }

template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator<<(set_adaptor<Bits, Own> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c <<= n; } { auto nrv = lhs; nrv <<= n; return nrv; }
template<class Bits, ownership Own> [[nodiscard]] constexpr auto operator>>(set_adaptor<Bits, Own> const& lhs, std::size_t n) noexcept(set_adaptor<Bits, Own>::has_static_width) -> set_adaptor<Bits, Own> requires (owns(Own)) and requires (set_adaptor<Bits, Own> c) { c >>= n; } { auto nrv = lhs; nrv >>= n; return nrv; }
// NOLINTEND(readability-redundant-parentheses)

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright, so a pipeline takes it as it is; and borrowed, its iterators pointing at the storage and not at it.
template<class Bits>
inline constexpr bool enable_view<xstd::set_adaptor<Bits, xstd::ownership::refers>> = true;

template<class Bits>
inline constexpr bool enable_borrowed_range<xstd::set_adaptor<Bits, xstd::ownership::refers>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// Owned or viewed, as std::string_view hashes and std::set does not.
template<class Bits, xstd::ownership Own>
struct hash<xstd::set_adaptor<Bits, Own>>
{
        [[nodiscard]] constexpr auto operator()(xstd::set_adaptor<Bits, Own> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

}       // namespace std
// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload, which cannot hash the proxy the set iterator returns.
namespace boost::container_hash {

template<class Bits, xstd::ownership Own>
struct is_range<xstd::set_adaptor<Bits, Own>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_SET_ADAPTOR_HPP
