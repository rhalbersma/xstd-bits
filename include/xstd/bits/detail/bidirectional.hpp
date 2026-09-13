//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
#define XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP

#include <xstd/bits/detail/zero_width.hpp> // zero_width
#include <cassert>                         // assert
#include <cstddef>                         // ptrdiff_t, size_t
#include <format>                          // formatter
#include <iterator>                        // bidirectional_iterator_tag
#include <type_traits>                     // is_class_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through the storage alone. [design.md#the-iterator-is-the-primitive] [design.md#why-nested]
namespace xstd::detail::bits {

template<class Bits> class bidirectional_bit_iterator;
template<class Bits> class bidirectional_bit_reference;

// A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through. [design.md#read-only-set-proxy]
template<class Bits>
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
        using reference         = bidirectional_bit_reference<Bits>;

        [[nodiscard]] constexpr bidirectional_bit_iterator() noexcept = default;

        // Public, so an owner or a view constructs one without befriending it: the dependency runs one way. [design.md#the-iterator-is-the-primitive]
        [[nodiscard]] constexpr bidirectional_bit_iterator(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A zero width has one position, so every iterator over it is the same one; said outright, every loop an optimizer sees into stops before its first step, which no spelling of the step itself achieved. [design.md#degenerate-widths]
        [[nodiscard]] friend constexpr auto operator==(bidirectional_bit_iterator lhs, bidirectional_bit_iterator rhs) noexcept
                -> bool
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                if constexpr (zero_width<Bits>) {
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

        // Both steps on the storage, guarded at a zero width rather than asking it: the exclusive scans take a position
        // as a precondition and a zero width has none to give, so they assert there. The trait's scans tested this first
        // and never reached the storage; the guard is what that test was, and it is load-bearing. [design.md#degenerate-widths]
        constexpr auto operator++() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        m_idx = m_ptr->exclusive_find_next(m_idx);
                }
                return *this;
        }

        constexpr auto operator--() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        m_idx = m_ptr->exclusive_find_prev(m_idx);
                }
                return *this;
        }

        constexpr auto operator++(int) noexcept -> bidirectional_bit_iterator { auto nrv = *this; ++*this; return nrv; }
        constexpr auto operator--(int) noexcept -> bidirectional_bit_iterator { auto nrv = *this; --*this; return nrv; }
};

// The key at a position, arriving by conversion; & hands the iterator back, so the pair round-trips. [design.md#read-only-set-proxy]
template<class Bits>
class bidirectional_bit_reference
{
        using bits_type = std::remove_const_t<Bits>;

        bits_type const* m_ptr;
        std::size_t m_idx;

public:
        using value_type = std::size_t;
        using iterator   = bidirectional_bit_iterator<Bits>;

        [[nodiscard]] constexpr bidirectional_bit_reference(bits_type const* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is. [design.md#the-proxy-copies-the-handle]
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


// std::format over the containers, which needs nothing said about the containers themselves. [design.md#formatting-the-proxies] [design.md#clang-tidy-false-positives]
template<class Bits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::detail::bits::bidirectional_bit_reference<Bits>, CharT>
:
        std::formatter<std::size_t, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::detail::bits::bidirectional_bit_reference<Bits> ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend. [design.md#the-one-adl-exception]
                return std::formatter<std::size_t, CharT>::format(format_as(ref), ctx);
        }
};

#endif  // XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
