//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BASIC_BIT_SEQUENCE_HPP
#define XSTD_BITS_BASIC_BIT_SEQUENCE_HPP

#include <xstd/bits/bit_proxy.hpp>  // bit_sequence_iterator, bit_sequence_reference
#include <xstd/bits/bit_traits.hpp> // bit_storage, bit_traits, block_readable
#include <xstd/bits/ownership.hpp>  // ownership, owns
#include <algorithm>                // lexicographical_compare_three_way
#include <cassert>                  // assert
#include <compare>                  // strong_ordering
#include <concepts>                 // convertible_to, swappable
#include <cstddef>                  // ptrdiff_t, size_t
#include <format>                   // format
#include <iterator>                 // make_reverse_iterator, reverse_iterator
#include <ranges>                   // swap
#include <source_location>          // source_location
#include <stdexcept>                // out_of_range
#include <type_traits>              // conditional_t, is_nothrow_swappable_v, remove_const_t, remove_reference_t
#include <utility>                  // as_const, declval

// The sequence reading, [array] over any Bits with a door, owning it or referring to it. [design.md#the-three-adaptors]
namespace xstd {

template<class Bits, ownership Own, bool Windowed, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>>
class basic_bit_sequence
{
        // A window stores an iterator and a size rather than a pointer, and arrives with subspan. [design.md#the-three-adaptors]
        static_assert(not Windowed, "windowed sequences arrive with subspan");

        static constexpr bool is_owner = owns(Own);

        using bits_type = std::remove_const_t<Bits>;

        std::conditional_t<is_owner, Bits, Bits*> m_bits;

        // One accessor: (self.m_bits) propagates the owner's const, *self.m_bits keeps the view shallow. [design.md#ownership-is-not-an-axis]
        [[nodiscard]] constexpr auto storage(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return (self.m_bits);
                } else {
                        return *self.m_bits;
                }
        }

        // What the accessor hands a given self, const included: the iterator and the proxy are spelled over exactly that.
        template<class Self>
        using storage_t = std::remove_reference_t<decltype(std::declval<Self>().storage())>;

        template<class Self> using iterator_t  = bit_sequence_iterator <storage_t<Self>, Traits>;
        template<class Self> using reference_t = bit_sequence_reference<storage_t<Self>, Traits>;

        // Still published for sequence_view and block_range, until the rewire routes them through the door. [design.md#the-iterator-is-the-primitive]
        [[nodiscard]] friend constexpr auto block_count(basic_bit_sequence const& c) noexcept -> std::size_t requires block_readable<Traits, bits_type> { return Traits::num_blocks(c.storage()); }
        [[nodiscard]] friend constexpr auto block_at(basic_bit_sequence const& c, std::size_t i) noexcept requires block_readable<Traits, bits_type> { return Traits::block(c.storage(), i); }

        [[nodiscard]] friend constexpr auto find_first(basic_bit_sequence const&)                  noexcept -> std::size_t { return 0UZ; }
        [[nodiscard]] friend constexpr auto find_last (basic_bit_sequence const& c)                noexcept -> std::size_t { return Traits::size(c.storage()); }
        [[nodiscard]] friend constexpr auto find_at   (basic_bit_sequence const& c, std::size_t n) noexcept -> bool        { return Traits::at(c.storage(), n); }
        friend constexpr void assign_at(basic_bit_sequence& c, std::size_t n, bool value) noexcept requires requires { Traits::assign(c.storage(), n, value); } { Traits::assign(c.storage(), n, value); }

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

        // construct/copy/destroy; an owner is built the way std::array is, a view only from what it views.
        [[nodiscard]] constexpr basic_bit_sequence() noexcept requires is_owner = default;

        [[nodiscard]] constexpr explicit basic_bit_sequence(Bits& c) noexcept
                requires (not is_owner)
        :
                m_bits(&c)
        {}

        constexpr void fill(this auto&& self, value_type const& u) noexcept
                requires requires { Traits::fill(self.storage(), u); }
        {
                Traits::fill(self.storage(), u);
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr void swap(basic_bit_sequence& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        // iterators, spelled over what the accessor hands this self: deep const for an owner, shallow for a view.
        [[nodiscard]] constexpr auto begin (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), 0UZ }; }
        [[nodiscard]] constexpr auto end   (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), Traits::size(self.storage()) }; }
        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept { return std::make_reverse_iterator(self.end());   }
        [[nodiscard]] constexpr auto rend  (this auto&& self) noexcept { return std::make_reverse_iterator(self.begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept -> const_iterator         { return { &std::as_const(storage()), 0UZ }; }
        [[nodiscard]] constexpr auto cend()    const noexcept -> const_iterator         { return { &std::as_const(storage()), Traits::size(storage()) }; }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cend());   }
        [[nodiscard]] constexpr auto crend()   const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cbegin()); }

        // capacity
        [[nodiscard]] constexpr auto    empty() const noexcept -> bool      { return size() == 0UZ; }
        [[nodiscard]] constexpr auto     size() const noexcept -> size_type { return Traits::size(storage()); }
        [[nodiscard]] constexpr auto max_size() const noexcept -> size_type { return size(); }

        // element access, [] unchecked and at() throwing as [array] has them. [design.md#asking-is-total]
        [[nodiscard]] constexpr auto operator[](this auto&& self, size_type n) noexcept
                -> reference_t<decltype(self)>
        {
                assert(n < self.size());
                return { &self.storage(), n };
        }

        [[nodiscard]] constexpr auto at(this auto&& self, size_type n)
                -> reference_t<decltype(self)>
        {
                if (n < self.size()) {
                        return { &self.storage(), n };
                }
                throw out_of_range(n, self.size());
        }

        [[nodiscard]] constexpr auto front(this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), 0UZ }; }
        [[nodiscard]] constexpr auto back (this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), self.size() - 1UZ }; }

        // The owner's alone, following span: a handle declines to say whether it compares its referent or its contents. [design.md#views-follow-their-precedent]
        [[nodiscard]] friend constexpr auto operator==(basic_bit_sequence const& x, basic_bit_sequence const& y) noexcept
                -> bool
                requires is_owner and requires { { x.storage() == y.storage() } -> std::convertible_to<bool>; }
        {
                return x.storage() == y.storage();
        }

        [[nodiscard]] friend constexpr auto operator<=>(basic_bit_sequence const& x, basic_bit_sequence const& y) noexcept
                -> std::strong_ordering
                requires is_owner
        {
                if constexpr (requires { Traits::sequence_three_way(x.storage(), y.storage()); }) {
                        return Traits::sequence_three_way(x.storage(), y.storage());
                } else {
                        return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end());
                }
        }

        // Bulk, on the storage's own spelling: on packed bits the pointwise operation and the set operation are one instruction. [design.md#what-the-door-reconciles]
        constexpr auto operator&=(this auto&& self, basic_bit_sequence const& other) noexcept -> auto& requires requires { self.storage() &= other.storage(); } { self.storage() &= other.storage(); return self; }
        constexpr auto operator|=(this auto&& self, basic_bit_sequence const& other) noexcept -> auto& requires requires { self.storage() |= other.storage(); } { self.storage() |= other.storage(); return self; }
        constexpr auto operator^=(this auto&& self, basic_bit_sequence const& other) noexcept -> auto& requires requires { self.storage() ^= other.storage(); } { self.storage() ^= other.storage(); return self; }
        constexpr auto operator-=(this auto&& self, basic_bit_sequence const& other) noexcept -> auto& requires requires { self.storage() -= other.storage(); } { self.storage() -= other.storage(); return self; }

        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept -> auto& requires requires { self.storage() <<= n; } { self.storage() <<= n; return self; }
        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept -> auto& requires requires { self.storage() >>= n; } { self.storage() >>= n; return self; }

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

// A view deduces the constness of what it views, the way span<T> and span<T const> do.
template<class Bits>
basic_bit_sequence(Bits&) -> basic_bit_sequence<Bits, ownership::refers, false>;

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.
template<class Bits, ownership Own, bool Windowed, class Traits>
constexpr void swap(basic_bit_sequence<Bits, Own, Windowed, Traits>& x, basic_bit_sequence<Bits, Own, Windowed, Traits>& y) noexcept(noexcept(x.swap(y)))
        requires (owns(Own))
{
        x.swap(y);
}
// NOLINTEND(readability-redundant-parentheses)

}       // namespace xstd

#endif  // XSTD_BITS_BASIC_BIT_SEQUENCE_HPP
