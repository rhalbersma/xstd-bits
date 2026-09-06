//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_PROXY_HPP
#define XSTD_BITS_BIT_PROXY_HPP

#include <xstd/bits/bit_traits.hpp> // bit_storage, bit_traits, find_next, find_prev, zero_width
#include <cassert>                  // assert
#include <compare>                  // strong_ordering
#include <concepts>                 // same_as
#include <cstddef>                  // ptrdiff_t, size_t
#include <iterator>                 // bidirectional_iterator_tag, random_access_iterator_tag
#include <type_traits>              // is_class_v, is_const_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterators are the primitive: a pointer and a position, reaching the bits through the door alone. [design.md#the-iterator-is-the-primitive]
namespace xstd {

template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bit_set_iterator;
template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bit_set_reference;
template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bit_sequence_iterator;
template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bit_sequence_reference;

// A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through. [design.md#read-only-set-proxy]
template<class Bits, bit_storage<Bits> Traits>
class bit_set_iterator
{
        using bits_type = std::remove_const_t<Bits>;

        bits_type const* m_ptr{};
        std::size_t m_idx{};

public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = std::size_t;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = bit_set_reference<Bits, Traits>;

        [[nodiscard]] constexpr bit_set_iterator() noexcept = default;

        // Public, so an owner or a view constructs one without befriending it: the dependency runs one way. [design.md#the-iterator-is-the-primitive]
        [[nodiscard]] constexpr bit_set_iterator(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A zero width has one position, so every iterator over it is the same one; said outright, every loop an optimizer
        // sees into stops before its first step, which no spelling of the step itself achieved. [design.md#degenerate-widths]
        [[nodiscard]] friend constexpr auto operator==(bit_set_iterator lhs, bit_set_iterator rhs) noexcept
                -> bool
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                if constexpr (detail::bits::zero_width<Traits>) {
                        return true;
                } else {
                        return lhs.m_idx == rhs.m_idx;
                }
        }

        [[nodiscard]] constexpr auto operator*() const noexcept
                -> reference
        {
                assert(m_ptr != nullptr);
                return { m_ptr, m_idx };
        }

        // Both steps through the door, native or synthesized as the specialization decides. [design.md#detection-by-absence]
        constexpr auto operator++() noexcept
                -> bit_set_iterator&
        {
                assert(m_ptr != nullptr);
                m_idx = detail::bits::find_next<Traits>(*m_ptr, m_idx);
                return *this;
        }

        constexpr auto operator--() noexcept
                -> bit_set_iterator&
        {
                assert(m_ptr != nullptr);
                m_idx = detail::bits::find_prev<Traits>(*m_ptr, m_idx);
                return *this;
        }

        constexpr auto operator++(int) noexcept -> bit_set_iterator { auto nrv = *this; ++*this; return nrv; }
        constexpr auto operator--(int) noexcept -> bit_set_iterator { auto nrv = *this; --*this; return nrv; }
};

// The key at a position, arriving by conversion; & hands the iterator back, so the pair round-trips. [design.md#read-only-set-proxy]
template<class Bits, bit_storage<Bits> Traits>
class bit_set_reference
{
        using bits_type = std::remove_const_t<Bits>;

        bits_type const* m_ptr;
        std::size_t m_idx;

public:
        using value_type = std::size_t;
        using iterator   = bit_set_iterator<Bits, Traits>;

        [[nodiscard]] constexpr bit_set_reference(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is.
        constexpr bit_set_reference(bit_set_reference const&) noexcept = default;
        constexpr auto operator=(bit_set_reference const&) -> bit_set_reference& = delete;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return { m_ptr, m_idx };
        }

        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept  // NOLINT(misc-explicit-constructor)
        {
                return m_idx;
        }

        // A strong index type initializes from *it in one step; one with an explicit constructor takes the size_t route. [design.md#read-only-set-proxy]
        template<class T>
        [[nodiscard]] constexpr explicit(false) operator T() const noexcept(std::is_nothrow_constructible_v<T, value_type>)  // NOLINT(misc-explicit-constructor)
                requires std::is_class_v<T> and std::is_convertible_v<value_type, T>
        {
                return m_idx;
        }

        // fmt's protocol, found by ADL on the proxy: the same value the conversion yields.
        [[nodiscard]] friend constexpr auto format_as(bit_set_reference ref) noexcept
                -> value_type
        {
                return ref.m_idx;
        }
};

// A position in the sequence reading; const Bits is the const iterator, the old IsConst bool folded into the type.
template<class Bits, bit_storage<Bits> Traits>
class bit_sequence_iterator
{
        Bits* m_ptr{};
        std::size_t m_idx{};

        // The const twin, whose conversion below reads these members; naming itself where Bits is already const.
        friend class bit_sequence_iterator<Bits const, Traits>;

public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = bit_sequence_reference<Bits, Traits>;

        [[nodiscard]] constexpr bit_sequence_iterator() noexcept = default;

        [[nodiscard]] constexpr bit_sequence_iterator(Bits* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A mutable iterator converts to its const twin, as a container's iterator converts to its const_iterator.
        template<class Mutable>
                requires std::is_const_v<Bits> and std::same_as<Mutable const, Bits>
        [[nodiscard]] constexpr explicit(false) bit_sequence_iterator(bit_sequence_iterator<Mutable, Traits> other) noexcept  // NOLINT(misc-explicit-constructor)
        :
                m_ptr(other.m_ptr),
                m_idx(other.m_idx)
        {}

        [[nodiscard]] friend constexpr auto operator==(bit_sequence_iterator lhs, bit_sequence_iterator rhs) noexcept
                -> bool
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return lhs.m_idx == rhs.m_idx;
        }

        [[nodiscard]] friend constexpr auto operator<=>(bit_sequence_iterator lhs, bit_sequence_iterator rhs) noexcept
                -> std::strong_ordering
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return lhs.m_idx <=> rhs.m_idx;
        }

        [[nodiscard]] constexpr auto operator*() const noexcept
                -> reference
        {
                assert(m_ptr != nullptr);
                return { m_ptr, m_idx };
        }

        constexpr auto operator++() noexcept -> bit_sequence_iterator& { ++m_idx; return *this; }
        constexpr auto operator--() noexcept -> bit_sequence_iterator& { --m_idx; return *this; }

        constexpr auto operator++(int) noexcept -> bit_sequence_iterator { auto nrv = *this; ++*this; return nrv; }
        constexpr auto operator--(int) noexcept -> bit_sequence_iterator { auto nrv = *this; --*this; return nrv; }

        constexpr auto operator+=(difference_type n) noexcept -> bit_sequence_iterator& { m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) + n); return *this; }
        constexpr auto operator-=(difference_type n) noexcept -> bit_sequence_iterator& { m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) - n); return *this; }

        [[nodiscard]] friend constexpr auto operator+(bit_sequence_iterator lhs, difference_type n) noexcept -> bit_sequence_iterator { auto nrv = lhs; nrv += n; return nrv; }
        [[nodiscard]] friend constexpr auto operator+(difference_type n, bit_sequence_iterator rhs) noexcept -> bit_sequence_iterator { auto nrv = rhs; nrv += n; return nrv; }
        [[nodiscard]] friend constexpr auto operator-(bit_sequence_iterator lhs, difference_type n) noexcept -> bit_sequence_iterator { auto nrv = lhs; nrv -= n; return nrv; }

        [[nodiscard]] friend constexpr auto operator-(bit_sequence_iterator lhs, bit_sequence_iterator rhs) noexcept
                -> difference_type
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return static_cast<difference_type>(lhs.m_idx) - static_cast<difference_type>(rhs.m_idx);
        }

        [[nodiscard]] constexpr auto operator[](difference_type n) const noexcept
                -> reference
        {
                return *(*this + n);
        }

        // The one ADL exception: std::ranges' own protocol, which is how sort and swap_ranges reach a proxy. [design.md#the-one-adl-exception]
        [[nodiscard]] friend constexpr auto iter_move(bit_sequence_iterator it) noexcept
                -> value_type
        {
                return *it;
        }

        friend constexpr void iter_swap(bit_sequence_iterator x, bit_sequence_iterator y) noexcept
                requires (not std::is_const_v<Bits>)
        {
                bool const t = *x;
                *x = *y;
                *y = t;
        }
};

// A proxy bool assigning back through the door; std::vector<bool>::reference is the precedent for the const-qualified assignment, and nothing more is borrowed: no flip, no ~.
template<class Bits, bit_storage<Bits> Traits>
class bit_sequence_reference
{
        Bits* m_ptr;
        std::size_t m_idx;

        // Writable where Bits is not const and the door has an entry to write through; a floor-only type reads only.
        static constexpr bool is_writable = not std::is_const_v<Bits> and requires (Bits& c, std::size_t n, bool value) { Traits::assign(c, n, value); };

public:
        using value_type = bool;
        using iterator   = bit_sequence_iterator<Bits, Traits>;

        [[nodiscard]] constexpr bit_sequence_reference(Bits* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return { m_ptr, m_idx };
        }

        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept  // NOLINT(misc-explicit-constructor)
        {
                return Traits::at(*m_ptr, m_idx);
        }

        template<class T>
        [[nodiscard]] constexpr explicit(false) operator T() const noexcept(std::is_nothrow_constructible_v<T, value_type>)  // NOLINT(misc-explicit-constructor)
                requires std::is_class_v<T> and std::is_convertible_v<value_type, T>
        {
                return Traits::at(*m_ptr, m_idx);
        }

        // const-qualified and returning a const reference, the proxy shape P2321R2 gave std::vector<bool>::reference.
        constexpr auto operator=(bool value) const noexcept  // NOLINT(misc-unconventional-assign-operator)
                -> bit_sequence_reference const&
                requires is_writable
        {
                Traits::assign(*m_ptr, m_idx, value);
                return *this;
        }

        // Assigns the bit, not the proxy: rebinding would break the swaps below. [design.md#clang-tidy-false-positives]
        constexpr auto operator=(bit_sequence_reference const& other) const noexcept  // NOLINT(misc-unconventional-assign-operator,bugprone-unhandled-self-assignment)
                -> bit_sequence_reference const&
                requires is_writable
        {
                return *this = static_cast<bool>(other);
        }

        // The pre-ranges spelling of iter_swap, for std::swap and the algorithms still built on it. [design.md#the-one-adl-exception]
        friend constexpr void swap(bit_sequence_reference x, bit_sequence_reference y) noexcept requires is_writable { bool const t = x; x = y; y = t; }
        friend constexpr void swap(bit_sequence_reference x, bool& y)                 noexcept requires is_writable { bool const t = x; x = y; y = t; }
        friend constexpr void swap(bool& x, bit_sequence_reference y)                 noexcept requires is_writable { bool const t = x; x = y; y = t; }

        [[nodiscard]] friend constexpr auto format_as(bit_sequence_reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

}       // namespace xstd

#endif  // XSTD_BITS_BIT_PROXY_HPP
