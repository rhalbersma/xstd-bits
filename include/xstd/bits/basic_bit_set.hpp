//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BASIC_BIT_SET_HPP
#define XSTD_BITS_BASIC_BIT_SET_HPP

#include <boost/hash2/hash_append.hpp> // hash_append
#include <xstd/bits/bit_proxy.hpp>     // bit_set_iterator, bit_set_reference
#include <xstd/bits/bit_traits.hpp>    // bit_storage, bit_traits, count, find_first, find_next, find_prev, static_bit_extent
#include <xstd/bits/ownership.hpp>     // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership, owns
#include <algorithm>                   // lexicographical_compare_three_way
#include <cassert>                     // assert
#include <compare>                     // strong_ordering
#include <concepts>                    // constructible_from, swappable
#include <cstddef>                     // ptrdiff_t, size_t
#include <functional>                  // less
#include <initializer_list>            // initializer_list
#include <iterator>                    // input_iterator, iter_reference_t, make_reverse_iterator, reverse_iterator, sentinel_for
#include <limits>                      // numeric_limits
#include <ranges>                      // begin, enable_borrowed_range, enable_view, end, input_range, range_reference_t, from_range_t, swap
#include <type_traits>                 // conditional_t, is_nothrow_swappable_v, remove_const_t, remove_reference_t
#include <utility>                     // forward, pair

// The set reading, [set] over any Bits with a bit_traits specialization, owning it or referring to it. [design.md#the-three-adaptors]
namespace xstd {

template<class Bits, ownership Own, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>>
class basic_bit_set
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
        template<class B, ownership O, bit_storage<B> T>         friend class basic_bit_set;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class basic_bit_sequence;

        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, basic_bit_set const* v) noexcept
        {
                boost::hash2::hash_append(h, f, v->storage());
        }

public:
        // types
        using key_type               = std::size_t;
        using key_compare            = std::less<key_type>;
        using value_type             = key_type;
        using value_compare          = key_compare;
        using traits_type            = Traits;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = bit_set_reference<Bits, Traits>;
        using const_reference        = reference;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = bit_set_iterator<Bits, Traits>;
        using const_iterator         = iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy; an owner is built the way std::set is, a view only from what it views.
        [[nodiscard]] constexpr basic_bit_set() noexcept requires is_owner = default;

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires is_owner and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr basic_bit_set(I first, S last)
        {
                insert(first, last);
        }

        template<std::ranges::input_range R>
                requires is_owner and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr basic_bit_set(std::from_range_t, R&& rg)
        {
                insert(std::ranges::begin(rg), std::ranges::end(rg));
        }

        [[nodiscard]] constexpr basic_bit_set(std::initializer_list<value_type> il)
                requires is_owner
        {
                insert(il.begin(), il.end());
        }

        [[nodiscard]] constexpr explicit basic_bit_set(Bits& c) noexcept
                requires (not is_owner)
        :
                m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. [design.md#views-over-owners]
        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit basic_bit_set(Owner& c) noexcept
                requires (not is_owner)
        :
                m_bits(&c.m_bits)
        {}

        constexpr auto operator=(std::initializer_list<value_type> il)
                -> basic_bit_set&
                requires is_owner
        {
                clear();
                insert(il.begin(), il.end());
                return *this;
        }

        // The storage's own equality, which every storage in the tree has; ordering is the trait's entry, or the invariant it must satisfy. [design.md#the-ordering-invariant]
        [[nodiscard]] friend constexpr auto operator==(basic_bit_set const& x, basic_bit_set const& y) noexcept
                -> bool
                requires requires { { x.storage() == y.storage() } -> std::convertible_to<bool>; }
        {
                return x.storage() == y.storage();
        }

        [[nodiscard]] friend constexpr auto operator<=>(basic_bit_set const& x, basic_bit_set const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (requires { Traits::set_three_way(x.storage(), y.storage()); }) {
                        return Traits::set_three_way(x.storage(), y.storage());
                } else {
                        return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                }
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

        // capacity; a bitset's count() is a set's size(), and a static width is the set's max_size(), a dynamic one grows to the last addressable position.
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return begin() == end(); }
        [[nodiscard]] constexpr auto full()  const noexcept -> bool { return size() == max_size(); }

        [[nodiscard]] constexpr auto size() const noexcept -> size_type { return detail::bits::count<Traits>(storage()); }

        [[nodiscard]] static constexpr auto max_size() noexcept
                -> size_type
        {
                if constexpr (static_bit_extent<Traits, bits_type>) {
                        return Traits::extent;
                } else {
                        // n + 1 must be addressable: the one position ruled out is the one whose successor wraps to zero. [design.md#asking-is-total]
                        return std::numeric_limits<size_type>::max() - 1UZ;
                }
        }

        // element access, both with a non-empty set as their precondition.
        [[nodiscard]] constexpr auto front() const noexcept -> const_reference { return *begin(); }
        [[nodiscard]] constexpr auto back()  const noexcept -> const_reference { return { &storage(), detail::bits::find_prev<Traits>(storage(), Traits::size(storage())) }; }

        // modifiers; each writes through Traits, so each exists exactly where Traits lets this handle write. [design.md#ownership-is-not-an-axis]
        template<class... Args>
        constexpr auto emplace(this auto&& self, Args&&... args)
                -> std::pair<iterator, bool>
                requires (sizeof...(args) == 1) and requires { Traits::insert(self.storage(), 0UZ); }
        {
                return self.do_insert(value_type(std::forward<Args>(args)...));
        }

        template<class... Args>
        constexpr auto emplace_hint(this auto&& self, const_iterator position, Args&&... args)
                -> iterator
                requires (sizeof...(args) == 1) and requires { Traits::insert(self.storage(), 0UZ); }
        {
                return self.do_insert(position, value_type(std::forward<Args>(args)...));
        }

        // [set]'s two overloads by value: a key is a size_t, and there is nothing to move.
        constexpr auto insert(this auto&& self, value_type x) -> std::pair<iterator, bool> requires requires { Traits::insert(self.storage(), x); } { return self.do_insert(x); }
        constexpr auto insert(this auto&& self, const_iterator position, value_type x) -> iterator requires requires { Traits::insert(self.storage(), x); } { return self.do_insert(position, x); }

        template<std::input_iterator I, std::sentinel_for<I> S>
        constexpr void insert(this auto&& self, I first, S last)
                requires std::constructible_from<value_type, std::iter_reference_t<I>> and requires { Traits::insert(self.storage(), 0UZ); }
        {
                for (; first != last; ++first) {
                        Traits::insert(self.storage(), static_cast<value_type>(*first));
                }
        }

        template<std::ranges::input_range R>
        constexpr void insert_range(this auto&& self, R&& rg)
                requires std::constructible_from<value_type, std::ranges::range_reference_t<R>> and requires { Traits::insert(self.storage(), 0UZ); }
        {
                self.insert(std::ranges::begin(rg), std::ranges::end(rg));
        }

        constexpr void insert(this auto&& self, std::initializer_list<value_type> ilist)
                requires requires { Traits::insert(self.storage(), 0UZ); }
        {
                self.insert(ilist.begin(), ilist.end());
        }

        constexpr void fill(this auto&& self) noexcept
                requires requires { Traits::fill(self.storage(), true); }
        {
                Traits::fill(self.storage(), true);
        }

        // The successor first: exclusive_find_next never reads the position it steps from, but the order costs nothing and says so.
        constexpr auto erase(this auto&& self, const_iterator position) noexcept
                -> iterator
                requires requires { Traits::unchecked_assign(self.storage(), 0UZ, false); }
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
                requires requires { Traits::unchecked_assign(self.storage(), 0UZ, false); }
        {
                if (not self.contains(x)) {
                        return 0UZ;
                }
                Traits::unchecked_assign(self.storage(), x, false);
                return 1UZ;
        }

        constexpr auto erase(this auto&& self, const_iterator first, const_iterator last) noexcept
                -> iterator
                requires requires { Traits::unchecked_assign(self.storage(), 0UZ, false); }
        {
                while (first != last) {
                        Traits::unchecked_assign(self.storage(), *first++, false);
                }
                return last;
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr void swap(basic_bit_set& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        constexpr void clear(this auto&& self) noexcept
                requires requires { Traits::fill(self.storage(), false); }
        {
                Traits::fill(self.storage(), false);
        }

        constexpr void complement(this auto&& self, value_type x) noexcept
                requires requires { Traits::unchecked_assign(self.storage(), x, true); }
        {
                assert(x < Traits::size(self.storage()));
                Traits::unchecked_assign(self.storage(), x, not Traits::at(self.storage(), x));
        }

        // Bulk, on the storage's own spelling: what every storage agrees on is required of it, not reconciled. [design.md#what-the-trait-reconciles]
        constexpr void complement(this auto&& self) noexcept requires requires { self.storage().flip(); } { self.storage().flip(); }

        constexpr auto operator&=(this auto&& self, basic_bit_set const& other) noexcept -> auto& requires requires { self.storage() &= other.storage(); } { self.storage() &= other.storage(); return self; }
        constexpr auto operator|=(this auto&& self, basic_bit_set const& other) noexcept -> auto& requires requires { self.storage() |= other.storage(); } { self.storage() |= other.storage(); return self; }
        constexpr auto operator^=(this auto&& self, basic_bit_set const& other) noexcept -> auto& requires requires { self.storage() ^= other.storage(); } { self.storage() ^= other.storage(); return self; }
        constexpr auto operator-=(this auto&& self, basic_bit_set const& other) noexcept -> auto& requires requires { self.storage() -= other.storage(); } { self.storage() -= other.storage(); return self; }

        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept -> auto& requires requires { self.storage() <<= n; } { self.storage() <<= n; return self; }
        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept -> auto& requires requires { self.storage() >>= n; } { self.storage() >>= n; return self; }

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
        [[nodiscard]] constexpr auto is_subset_of(basic_bit_set const& other) const noexcept
                -> bool
        {
                if constexpr (requires { storage().is_subset_of(other.storage()); }) {
                        return storage().is_subset_of(other.storage());
                } else {
                        return (storage() & ~other.storage()).none();
                }
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(basic_bit_set const& other) const noexcept
                -> bool
        {
                if constexpr (requires { storage().is_proper_subset_of(other.storage()); }) {
                        return storage().is_proper_subset_of(other.storage());
                } else {
                        return is_subset_of(other) and *this != other;
                }
        }

        [[nodiscard]] constexpr auto intersects(basic_bit_set const& other) const noexcept
                -> bool
        {
                if constexpr (requires { storage().intersects(other.storage()); }) {
                        return storage().intersects(other.storage());
                } else {
                        return (storage() & other.storage()).any();
                }
        }

private:
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
template<class Bits>
basic_bit_set(Bits&) -> basic_bit_set<Bits, ownership::refers>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
basic_bit_set(Owner&) -> basic_bit_set<owned_bits_t<Owner>, ownership::refers, owned_traits_t<Owner>>;

// The owner's side of the protocol above.
template<class Bits, class Traits>
struct owned_storage<basic_bit_set<Bits, ownership::owns, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.
template<class Bits, ownership Own, class Traits>
constexpr void swap(basic_bit_set<Bits, Own, Traits>& x, basic_bit_set<Bits, Own, Traits>& y) noexcept(noexcept(x.swap(y)))
        requires (owns(Own))
{
        x.swap(y);
}

// 23.4.6.3 Erasure                                                [set.erasure]
template<class Bits, ownership Own, class Traits, class Predicate>
constexpr auto erase_if(basic_bit_set<Bits, Own, Traits>& c, Predicate pred)
        -> basic_bit_set<Bits, Own, Traits>::size_type
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

// The non-member forms copy, so they are the owner's alone: a copied view would write through to what it views.
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator~(basic_bit_set<Bits, Own, Traits> const& lhs) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c.complement(); } { auto nrv = lhs; nrv.complement(); return nrv; }

template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator&(basic_bit_set<Bits, Own, Traits> const& lhs, basic_bit_set<Bits, Own, Traits> const& rhs) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c &= c; } { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator|(basic_bit_set<Bits, Own, Traits> const& lhs, basic_bit_set<Bits, Own, Traits> const& rhs) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c |= c; } { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator^(basic_bit_set<Bits, Own, Traits> const& lhs, basic_bit_set<Bits, Own, Traits> const& rhs) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c ^= c; } { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator-(basic_bit_set<Bits, Own, Traits> const& lhs, basic_bit_set<Bits, Own, Traits> const& rhs) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c -= c; } { auto nrv = lhs; nrv -= rhs; return nrv; }

template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator<<(basic_bit_set<Bits, Own, Traits> const& lhs, std::size_t n) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c <<= n; } { auto nrv = lhs; nrv <<= n; return nrv; }
template<class Bits, ownership Own, class Traits> [[nodiscard]] constexpr auto operator>>(basic_bit_set<Bits, Own, Traits> const& lhs, std::size_t n) noexcept -> basic_bit_set<Bits, Own, Traits> requires (owns(Own)) and requires (basic_bit_set<Bits, Own, Traits> c) { c >>= n; } { auto nrv = lhs; nrv >>= n; return nrv; }
// NOLINTEND(readability-redundant-parentheses)

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright, so a pipeline takes it as it is; and borrowed, its iterators pointing at the storage and not at it. [design.md#views-follow-their-precedent]
template<class Bits, class Traits>
inline constexpr bool enable_view<xstd::basic_bit_set<Bits, xstd::ownership::refers, Traits>> = true;

template<class Bits, class Traits>
inline constexpr bool enable_borrowed_range<xstd::basic_bit_set<Bits, xstd::ownership::refers, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

#endif  // XSTD_BITS_BASIC_BIT_SET_HPP
