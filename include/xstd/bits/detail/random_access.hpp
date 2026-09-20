//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
#define XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP

#include <xstd/ints/concepts/integer.hpp> // integer
#include <cassert>                        // assert
#include <compare>                        // strong_ordering
#include <concepts>                       // same_as
#include <cstddef>                        // ptrdiff_t, size_t
#include <format>                         // formatter
#include <iterator>                       // random_access_iterator_tag
#include <type_traits>                    // is_class_v, is_const_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through the storage alone.
namespace xstd::detail::bits {

template<class Bits> class random_access_bit_iterator;
template<class Bits> class random_access_bit_reference;

// A position in the sequence reading; const Bits is the const iterator, the old IsConst bool folded into the type.
template<class Bits>
class random_access_bit_iterator
{
        Bits* m_ptr{};
        std::size_t m_idx{};

        // The const twin, whose conversion below reads these members; naming itself where Bits is already const.
        friend class random_access_bit_iterator<Bits const>;

public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = random_access_bit_reference<Bits>;

        [[nodiscard]] random_access_bit_iterator() noexcept = default;

        [[nodiscard]] constexpr random_access_bit_iterator(Bits* ptr, std::size_t idx) noexcept
            : m_ptr(ptr),
              m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A mutable iterator converts to its const twin, as a container's iterator converts to its const_iterator.
        template<class Mutable>
                requires std::is_const_v<Bits> and std::same_as<Mutable const, Bits>
        [[nodiscard]] constexpr explicit(false) random_access_bit_iterator(random_access_bit_iterator<Mutable> other) noexcept // NOLINT(misc-explicit-constructor)
            : m_ptr(other.m_ptr),
              m_idx(other.m_idx)
        {}

        [[nodiscard]] friend constexpr auto operator==(random_access_bit_iterator lhs, random_access_bit_iterator rhs) noexcept
                -> bool
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return lhs.m_idx == rhs.m_idx;
        }

        [[nodiscard]] friend constexpr auto operator<=>(random_access_bit_iterator lhs, random_access_bit_iterator rhs) noexcept
                -> std::strong_ordering
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return lhs.m_idx <=> rhs.m_idx;
        }

        // The position has to exist, which end()'s does not: this proxy reads and writes through the storage.
        [[nodiscard]] constexpr auto operator*() const noexcept
                -> reference
        {
                assert(m_ptr != nullptr);
                assert(m_idx < m_ptr->size());
                return {m_ptr, m_idx};
        }

        constexpr auto operator++() noexcept
                -> random_access_bit_iterator&
        {
                ++m_idx;
                return *this;
        }
        constexpr auto operator--() noexcept
                -> random_access_bit_iterator&
        {
                --m_idx;
                return *this;
        }

        constexpr auto operator++(int) noexcept
                -> random_access_bit_iterator
        {
                auto nrv = *this;
                ++*this;
                return nrv;
        }
        constexpr auto operator--(int) noexcept
                -> random_access_bit_iterator
        {
                auto nrv = *this;
                --*this;
                return nrv;
        }

        constexpr auto operator+=(difference_type n) noexcept
                -> random_access_bit_iterator&
        {
                m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) + n);
                return *this;
        }
        constexpr auto operator-=(difference_type n) noexcept
                -> random_access_bit_iterator&
        {
                m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) - n);
                return *this;
        }

        [[nodiscard]] friend constexpr auto operator+(random_access_bit_iterator lhs, difference_type n) noexcept
                -> random_access_bit_iterator
        {
                auto nrv = lhs;
                nrv += n;
                return nrv;
        }
        [[nodiscard]] friend constexpr auto operator+(difference_type n, random_access_bit_iterator rhs) noexcept
                -> random_access_bit_iterator
        {
                auto nrv = rhs;
                nrv += n;
                return nrv;
        }
        [[nodiscard]] friend constexpr auto operator-(random_access_bit_iterator lhs, difference_type n) noexcept
                -> random_access_bit_iterator
        {
                auto nrv = lhs;
                nrv -= n;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator-(random_access_bit_iterator lhs, random_access_bit_iterator rhs) noexcept
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

        // The one ADL exception: std::ranges' own protocol, which is how sort and swap_ranges reach a proxy.
        [[nodiscard]] friend constexpr auto iter_move(random_access_bit_iterator it) noexcept
                -> value_type
        {
                return *it;
        }

        friend constexpr auto iter_swap(random_access_bit_iterator x, random_access_bit_iterator y) noexcept
                -> void
                requires (not std::is_const_v<Bits>)
        {
                bool const t = *x;
                *x = *y;
                *y = t;
        }
};

// A proxy bool spelled as [vector.bool] spells std::vector<bool>::reference; operator~ is std::bitset's.
template<class Bits>
class random_access_bit_reference
{
        Bits* m_ptr;
        std::size_t m_idx;

        // Writable where Bits is not const: a const storage has no assign to reach.
        static constexpr bool is_writable = not std::is_const_v<Bits> and requires (Bits& c, std::size_t n, bool value) { c.assign(n, value); };

public:
        using value_type = bool;
        using iterator = random_access_bit_iterator<Bits>;

        [[nodiscard]] constexpr random_access_bit_reference(Bits* ptr, std::size_t idx) noexcept
            : m_ptr(ptr),
              m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // Said out loud: the assignments below are user-provided, which deprecates the implicit copy constructor.
        random_access_bit_reference(random_access_bit_reference const&) noexcept = default;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return {m_ptr, m_idx};
        }

        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return m_ptr->test(m_idx);
        }

        // Not to an integer, though, however class-shaped it is.
        template<class T>
        [[nodiscard]] constexpr explicit(false) operator T() const noexcept(std::is_nothrow_constructible_v<T, value_type>) // NOLINT(misc-explicit-constructor)
                requires std::is_class_v<T> and std::is_convertible_v<value_type, T> and (not xstd::integer<T>)
        {
                return m_ptr->test(m_idx);
        }

        // Exact matches, so a comparison never reaches for a conversion.
        [[nodiscard]] friend constexpr auto operator==(random_access_bit_reference lhs, random_access_bit_reference rhs) noexcept
                -> bool
        {
                return static_cast<value_type>(lhs) == static_cast<value_type>(rhs);
        }

        [[nodiscard]] friend constexpr auto operator==(random_access_bit_reference lhs, value_type rhs) noexcept
                -> bool
        {
                return static_cast<value_type>(lhs) == rhs;
        }

        // const-qualified and returning a const reference, the proxy shape P2321R2 gave std::vector<bool>::reference.
        constexpr auto operator=(bool value) const noexcept // NOLINT(misc-unconventional-assign-operator)
                -> random_access_bit_reference const&
                requires is_writable
        {
                m_ptr->assign(m_idx, value);
                return *this;
        }

        // Assigns the bit, not the proxy: rebinding would break the swaps below.
        constexpr auto operator=(random_access_bit_reference const& other) const noexcept // NOLINT(misc-unconventional-assign-operator,bugprone-unhandled-self-assignment)
                -> random_access_bit_reference const&
                requires is_writable
        {
                return *this = static_cast<bool>(other);
        }

        // [vector.bool] has required it of the proxy since C++98, where the const-qualified assignment is C++23.
        constexpr auto flip() const noexcept
                -> void
                requires is_writable
        {
                m_ptr->assign(m_idx, not static_cast<value_type>(*this));
        }

        // The pre-ranges spelling of iter_swap, for std::swap and the algorithms still built on it.
        friend constexpr auto swap(random_access_bit_reference x, random_access_bit_reference y) noexcept -> void
                requires is_writable
        {
                bool const t = x;
                x = y;
                y = t;
        }
        friend constexpr auto swap(random_access_bit_reference x, bool& y) noexcept -> void
                requires is_writable
        {
                bool const t = x;
                x = y;
                y = t;
        }
        friend constexpr auto swap(bool& x, random_access_bit_reference y) noexcept -> void
                requires is_writable
        {
                bool const t = x;
                x = y;
                y = t;
        }

        // What this proxy prints as, said once: our std::formatter calls it unqualified, and fmt finds it by ADL.
        [[nodiscard]] friend constexpr auto format_as(random_access_bit_reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

} // namespace xstd::detail::bits

// std::format over the containers, which needs nothing said about the containers themselves.
template<class Bits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::detail::bits::random_access_bit_reference<Bits>, CharT>
    : std::formatter<bool, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::detail::bits::random_access_bit_reference<Bits> ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend.
                return std::formatter<bool, CharT>::format(format_as(ref), ctx);
        }
};

#endif // XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
