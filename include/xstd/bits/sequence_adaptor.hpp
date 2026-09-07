//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SEQUENCE_ADAPTOR_HPP
#define XSTD_BITS_SEQUENCE_ADAPTOR_HPP

#include <boost/container_hash/is_range.hpp> // is_range
#include <boost/hash2/hash_append.hpp>       // hash_append_tag
#include <xstd/bits/bit_proxy.hpp>           // bit_sequence_iterator, bit_sequence_reference
#include <xstd/bits/bit_traits.hpp>          // bit_storage, bit_traits, static_bit_extent
#include <xstd/bits/detail/hash.hpp>         // hash_append_bits, std_hash
#include <xstd/bits/ownership.hpp>           // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership, owns
#include <algorithm>                         // lexicographical_compare_three_way
#include <cassert>                           // assert
#include <compare>                           // strong_ordering
#include <concepts>                          // convertible_to, swap, swappable
#include <cstddef>                           // ptrdiff_t, size_t
#include <format>                            // format
#include <functional>                        // hash
#include <initializer_list>                  // initializer_list
#include <iterator>                          // input_iterator, make_reverse_iterator, reverse_iterator, sentinel_for
#include <limits>                            // numeric_limits
#include <ranges>                            // begin, enable_borrowed_range, enable_view, end, from_range_t, input_range, range_reference_t
#include <source_location>                   // source_location
#include <span>                              // dynamic_extent
#include <stdexcept>                         // out_of_range
#include <type_traits>                       // conditional_t, false_type, is_nothrow_swappable_v, remove_const_t, remove_reference_t
#include <utility>                           // as_const, declval

// The sequence reading, [array] over any Bits with a bit_traits specialization, owning it or referring to it. [design.md#the-three-adaptors]
namespace xstd {

template<class Bits, ownership Own, bool Windowed, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>>
class sequence_adaptor
{
        static constexpr bool is_owner  = owns(Own);
        static constexpr bool is_window = Windowed;
        static_assert(not (is_owner and is_window), "a window views what another owns");

        using bits_type = std::remove_const_t<Bits>;

        // Growth is the owner's over storage that grows: a view must never resize what it does not own. [design.md#growth]
        static constexpr bool can_grow = is_owner and not static_bit_extent<Traits, bits_type> and requires (bits_type& b) { b.resize(0UZ, true); b.push_back(true); b.pop_back(); b.clear(); };

        // A window is what std::span stores, the pointer's role split over a pointer and a position because bits are not addressable: the iterator's two fields and a size. [design.md#windows]
        struct window
        {
                Bits* ptr;
                std::size_t offset;
                std::size_t size;
        };

        std::conditional_t<is_owner, Bits, std::conditional_t<is_window, window, Bits*>> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow, a window's pointer likewise. [design.md#ownership-is-not-an-axis]
        [[nodiscard]] constexpr auto storage(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return self.m_bits;
                } else if constexpr (is_window) {
                        return *self.m_bits.ptr;
                } else {
                        return *self.m_bits;
                }
        }

        // Where this sequence starts in the storage: zero but for a window, so every position below is offset once, here. [design.md#windows]
        [[nodiscard]] constexpr auto offset() const noexcept
                -> std::size_t
        {
                if constexpr (is_window) {
                        return m_bits.offset;
                } else {
                        return 0UZ;
                }
        }

        // The window's constructor, which first, last and subspan call and nothing else does.
        [[nodiscard]] constexpr sequence_adaptor(Bits* ptr, std::size_t offset, std::size_t size) noexcept
                requires is_window
        :
                m_bits{ ptr, offset, size }
        {}

        // What the accessor hands a given self, const included: the iterator and the proxy are spelled over exactly that.
        template<class Self>
        using storage_t = std::remove_reference_t<decltype(std::declval<Self>().storage())>;

        template<class Self> using iterator_t  = bit_sequence_iterator <storage_t<Self>, Traits>;
        template<class Self> using reference_t = bit_sequence_reference<storage_t<Self>, Traits>;

        // Either reading's view refers into this owner's storage, and nothing else outside does. [design.md#views-over-owners]
        template<class B, ownership O, bit_storage<B> T>         friend class set_adaptor;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class sequence_adaptor;

        // The value under the sequence reading, the owner's alone as == is: a view follows span and hashes no more than it compares. [design.md#the-hashing-invariant]
        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, sequence_adaptor const* v) noexcept
                requires is_owner
        {
                detail::bits::hash_append_bits<Traits>(h, f, v->storage());
        }

public:
        // types
        using value_type             = bool;
        using traits_type            = Traits;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = bit_sequence_reference<Bits, Traits>;
        using const_reference        = bit_sequence_reference<Bits const, Traits>;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = bit_sequence_iterator<Bits, Traits>;
        using const_iterator         = bit_sequence_iterator<Bits const, Traits>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy; an owner is built the way std::array is, or std::vector where the storage grows, a view only from what it views.
        [[nodiscard]] constexpr sequence_adaptor() noexcept requires is_owner = default;

        [[nodiscard]] constexpr explicit sequence_adaptor(size_type n)
                requires can_grow
        :
                m_bits(n)
        {}

        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value)
                requires can_grow
        :
                m_bits(n)
        {
                if (value) {
                        Traits::fill(m_bits, true);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr sequence_adaptor(I first, S last)
        {
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr sequence_adaptor(std::from_range_t, R&& rg)
        :
                sequence_adaptor(std::ranges::begin(rg), std::ranges::end(rg))
        {}

        [[nodiscard]] constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires can_grow
        :
                sequence_adaptor(il.begin(), il.end())
        {}

        constexpr auto operator=(std::initializer_list<value_type> il)
                -> sequence_adaptor&
                requires can_grow
        {
                assign(il.begin(), il.end());
                return *this;
        }

        // [sequence.reqmts]: assign in its three shapes, each a clear and a refill.
        constexpr void assign(size_type n, value_type const& value)
                requires can_grow
        {
                m_bits.clear();
                m_bits.resize(n, value);
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>>
        constexpr void assign(I first, S last)
        {
                m_bits.clear();
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        constexpr void assign(std::initializer_list<value_type> il)
                requires can_grow
        {
                assign(il.begin(), il.end());
        }

        [[nodiscard]] constexpr explicit sequence_adaptor(Bits& c) noexcept
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. [design.md#views-over-owners]
        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit sequence_adaptor(Owner& c) noexcept
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c.m_bits)
        {}

        // [span.sub]'s three, on a view and never on an owner, since std::array and std::vector have no subviews; asserting their preconditions, dynamic_extent reaching the end. [design.md#windows]
        using subspan_type = sequence_adaptor<Bits, ownership::refers, true, Traits>;

        [[nodiscard]] constexpr auto first(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return { &storage(), offset(), count };
        }

        [[nodiscard]] constexpr auto last(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return { &storage(), offset() + (size() - count), count };
        }

        [[nodiscard]] constexpr auto subspan(size_type off, size_type count = std::dynamic_extent) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(off <= size());
                assert(count == std::dynamic_extent or count <= size() - off);
                return { &storage(), offset() + off, count == std::dynamic_extent ? size() - off : count };
        }

        // Bulk on a window is the masked-block work that arrives with the blit; until then a window is read and written one position at a time. [design.md#windows]
        constexpr void fill(this auto&& self, value_type const& u) noexcept
                requires (not is_window) and requires { Traits::fill(self.storage(), u); }
        {
                Traits::fill(self.storage(), u);
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr void swap(sequence_adaptor& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        // iterators, spelled over what the accessor hands this self: deep const for an owner, shallow for a view.
        [[nodiscard]] constexpr auto begin (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), self.offset() }; }
        [[nodiscard]] constexpr auto end   (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), self.offset() + self.size() }; }
        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept { return std::make_reverse_iterator(self.end());   }
        [[nodiscard]] constexpr auto rend  (this auto&& self) noexcept { return std::make_reverse_iterator(self.begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept -> const_iterator         { return { &std::as_const(storage()), offset() }; }
        [[nodiscard]] constexpr auto cend()    const noexcept -> const_iterator         { return { &std::as_const(storage()), offset() + size() }; }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cend());   }
        [[nodiscard]] constexpr auto crend()   const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cbegin()); }

        // capacity; a static width is its own max_size, a growing one has the address space's, a window its own count.
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return size() == 0UZ; }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                if constexpr (is_window) {
                        return m_bits.size;
                } else {
                        return Traits::size(storage());
                }
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr (can_grow) {
                        return std::numeric_limits<size_type>::max();
                } else {
                        return size();
                }
        }

        // Growth, [vector]'s members over storage that spells them alike, so detected on the storage rather than reconciled by the trait. [design.md#growth]
        constexpr void resize(size_type n)                          requires can_grow { m_bits.resize(n); }
        constexpr void resize(size_type n, value_type const& value) requires can_grow { m_bits.resize(n, value); }
        constexpr void clear() noexcept                             requires can_grow { m_bits.clear(); }
        constexpr void push_back(value_type const& value)           requires can_grow { m_bits.push_back(value); }
        constexpr void pop_back() noexcept                          requires can_grow { m_bits.pop_back(); }

        constexpr auto emplace_back(value_type const& value)
                -> reference
                requires can_grow
        {
                m_bits.push_back(value);
                return back();
        }

        constexpr void reserve(size_type n)
                requires can_grow and requires (bits_type& b) { b.reserve(n); }
        {
                m_bits.reserve(n);
        }

        [[nodiscard]] constexpr auto capacity() const noexcept
                -> size_type
                requires can_grow and requires (bits_type const& b) { b.capacity(); }
        {
                return m_bits.capacity();
        }

        constexpr void shrink_to_fit()
                requires can_grow and requires (bits_type& b) { b.shrink_to_fit(); }
        {
                m_bits.shrink_to_fit();
        }

        // element access, [] unchecked and at() throwing as [array] has them. [design.md#asking-is-total]
        [[nodiscard]] constexpr auto operator[](this auto&& self, size_type n) noexcept
                -> reference_t<decltype(self)>
        {
                assert(n < self.size());
                return { &self.storage(), self.offset() + n };
        }

        [[nodiscard]] constexpr auto at(this auto&& self, size_type n)
                -> reference_t<decltype(self)>
        {
                if (n < self.size()) {
                        return { &self.storage(), self.offset() + n };
                }
                throw out_of_range(n, self.size());
        }

        [[nodiscard]] constexpr auto front(this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), self.offset() }; }
        [[nodiscard]] constexpr auto back (this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), self.offset() + self.size() - 1UZ }; }

        // The owner's alone, following span: a handle declines to say whether it compares its referent or its contents. Defaulted, the storage being the one member. [design.md#views-follow-their-precedent]
        [[nodiscard]] friend constexpr auto operator==(sequence_adaptor const& x, sequence_adaptor const& y) noexcept
                -> bool
                requires is_owner
        = default;

        [[nodiscard]] friend constexpr auto operator<=>(sequence_adaptor const& x, sequence_adaptor const& y) noexcept
                -> std::strong_ordering
                requires is_owner
        {
                if constexpr (requires { Traits::sequence_three_way(x.storage(), y.storage()); }) {
                        return Traits::sequence_three_way(x.storage(), y.storage());
                } else {
                        return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                }
        }

        // Bulk, on the storage's own spelling: on packed bits the pointwise operation and the set operation are one instruction; not on a window, whose blocks are not its own. [design.md#what-the-trait-reconciles]
        constexpr auto operator&=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() &= other.storage(); } { self.storage() &= other.storage(); return self; }
        constexpr auto operator|=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() |= other.storage(); } { self.storage() |= other.storage(); return self; }
        constexpr auto operator^=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() ^= other.storage(); } { self.storage() ^= other.storage(); return self; }
        constexpr auto operator-=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() -= other.storage(); } { self.storage() -= other.storage(); return self; }

        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept -> auto& requires (not is_window) and requires { self.storage() <<= n; } { self.storage() <<= n; return self; }
        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept -> auto& requires (not is_window) and requires { self.storage() >>= n; } { self.storage() >>= n; return self; }

private:
        static constexpr auto out_of_range(std::size_t n, std::size_t size, std::source_location const& loc = std::source_location::current())
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘n‘ is out of range [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), n, size
                        )
                );
        }
};

// A view deduces the constness of what it views, the way span<T> and span<T const> do; over an owner, of the storage it wraps.
template<class Bits>
sequence_adaptor(Bits&) -> sequence_adaptor<Bits, ownership::refers, false>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
sequence_adaptor(Owner&) -> sequence_adaptor<owned_bits_t<Owner>, ownership::refers, false, owned_traits_t<Owner>>;

// The owner's side of the protocol above.
template<class Bits, class Traits>
struct owned_storage<sequence_adaptor<Bits, ownership::owns, false, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.
template<class Bits, ownership Own, bool Windowed, class Traits>
constexpr void swap(sequence_adaptor<Bits, Own, Windowed, Traits>& x, sequence_adaptor<Bits, Own, Windowed, Traits>& y) noexcept(noexcept(x.swap(y)))
        requires (owns(Own))
{
        x.swap(y);
}
// NOLINTEND(readability-redundant-parentheses)

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright and borrowed, as set_adaptor's is. [design.md#views-follow-their-precedent]
template<class Bits, bool Windowed, class Traits>
inline constexpr bool enable_view<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed, Traits>> = true;

template<class Bits, bool Windowed, class Traits>
inline constexpr bool enable_borrowed_range<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// The owner hashes as std::vector<bool> does; a view no more than std::span does. [design.md#the-hashing-invariant]
template<class Bits, bool Windowed, class Traits>
struct hash<xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed, Traits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed, Traits> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

}       // namespace std
// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload, which cannot hash the proxy the iterator returns. [design.md#the-hashing-invariant]
namespace boost::container_hash {

template<class Bits, xstd::ownership Own, bool Windowed, class Traits>
struct is_range<xstd::sequence_adaptor<Bits, Own, Windowed, Traits>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_SEQUENCE_ADAPTOR_HPP
