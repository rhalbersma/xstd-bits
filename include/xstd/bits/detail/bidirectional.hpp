//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
#define XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP

#include <xstd/bits/bit_traits.hpp> // bit_storage, bit_traits, find_next, find_prev, zero_width
#include <cassert>                  // assert
#include <cstddef>                  // ptrdiff_t, size_t
#include <iterator>                 // bidirectional_iterator_tag
#include <type_traits>              // is_class_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through Traits alone. [design.md#the-iterator-is-the-primitive]
// The set reading's pair, named after the category its iterator models; the sequence reading's is random_access.hpp.
// The walks below stay qualified inside their own namespace: unqualified, the explicit template argument would drag
// ADL in with it, and the associated namespace of a std::bitset is std. [design.md#why-nested]
namespace xstd::detail::bits {

template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bidirectional_bit_iterator;
template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class bidirectional_bit_reference;

// A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through. [design.md#read-only-set-proxy]
template<class Bits, bit_storage<Bits> Traits>
class bidirectional_bit_iterator
{
        using bits_type = std::remove_const_t<Bits>;

        bits_type const* m_ptr{};
        std::size_t m_idx{};

public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = std::size_t;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = bidirectional_bit_reference<Bits, Traits>;

        [[nodiscard]] constexpr bidirectional_bit_iterator() noexcept = default;

        // Public, so an owner or a view constructs one without befriending it: the dependency runs one way. [design.md#the-iterator-is-the-primitive]
        [[nodiscard]] constexpr bidirectional_bit_iterator(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A zero width has one position, so every iterator over it is the same one; said outright, every loop an optimizer
        // sees into stops before its first step, which no spelling of the step itself achieved. [design.md#degenerate-widths]
        [[nodiscard]] friend constexpr auto operator==(bidirectional_bit_iterator lhs, bidirectional_bit_iterator rhs) noexcept
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

        // Both steps through the trait, native where its specialization declares the entry and synthesized where it does not. [design.md#detection-by-absence]
        constexpr auto operator++() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                m_idx = detail::bits::find_next<Traits>(*m_ptr, m_idx);
                return *this;
        }

        constexpr auto operator--() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                m_idx = detail::bits::find_prev<Traits>(*m_ptr, m_idx);
                return *this;
        }

        constexpr auto operator++(int) noexcept -> bidirectional_bit_iterator { auto nrv = *this; ++*this; return nrv; }
        constexpr auto operator--(int) noexcept -> bidirectional_bit_iterator { auto nrv = *this; --*this; return nrv; }
};

// The key at a position, arriving by conversion; & hands the iterator back, so the pair round-trips. [design.md#read-only-set-proxy]
template<class Bits, bit_storage<Bits> Traits>
class bidirectional_bit_reference
{
        using bits_type = std::remove_const_t<Bits>;

        bits_type const* m_ptr;
        std::size_t m_idx;

public:
        using value_type = std::size_t;
        using iterator   = bidirectional_bit_iterator<Bits, Traits>;

        [[nodiscard]] constexpr bidirectional_bit_reference(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is.
        // [design.md#the-proxy-copies-the-handle]
        constexpr bidirectional_bit_reference(bidirectional_bit_reference const&) noexcept = default;
        constexpr auto operator=(bidirectional_bit_reference const&) -> bidirectional_bit_reference& = delete;

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
        [[nodiscard]] friend constexpr auto format_as(bidirectional_bit_reference ref) noexcept
                -> value_type
        {
                return ref.m_idx;
        }
};

}       // namespace xstd::detail::bits

#endif  // XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
