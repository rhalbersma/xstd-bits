//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
#define XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP

#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container_type
#include <xstd/bits/detail/ownership.hpp>           // storage, window
#include <xstd/bits/detail/storage_ptr.hpp>         // storage_ptr_t
#include <cassert>                                  // assert
#include <compare>                                  // strong_ordering
#include <concepts>                                 // same_as
#include <cstddef>                                  // ptrdiff_t, size_t
#include <format>                                   // formatter
#include <iterator>                                 // random_access_iterator_tag
#include <type_traits>                              // is_const_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through the storage alone.
namespace xstd::bits::detail {

// Member templates on the value type alone: ADL associates its namespaces with the pair, and none of Bits'.
template<class Bits>
struct random_access
{
        using value_type = bool;

        template<class Value>
        class basic_iterator;

        template<class Value>
        class basic_reference;

        using iterator  = basic_iterator<value_type>;
        using reference = basic_reference<value_type>;
};

template<class Bits>
using random_access_bit_iterator = random_access<Bits>::iterator;

template<class Bits>
using random_access_bit_reference = random_access<Bits>::reference;

template<class>
inline constexpr bool is_random_access = false;

template<class Bits>
inline constexpr bool is_random_access<random_access<Bits>> = true;

// Recognized through the enclosing class it names, since no deduction reaches Bits through a nested class.
template<class R>
concept random_access_reference = is_random_access<typename R::enclosing_type> and std::same_as<R, typename R::enclosing_type::reference>;

template<bit_block_container_type Bits, storage Store, window W, class Derived, std::size_t E>
class sequence_adaptor;

// A position in the sequence reading; const Bits is the const iterator, the old IsConst bool folded into the type.
template<class Bits>
template<class Value>
class random_access<Bits>::basic_iterator
{
        // Value exists only to put the value type's namespaces among the associated ones: it is no second axis.
        static_assert(std::same_as<Value, random_access::value_type>);

        storage_ptr_t<Bits> m_ptr{};
        std::size_t m_idx{};

        // The const twin, whose conversion below reads these members; naming itself where Bits is already const.
        friend class random_access<Bits const>::template basic_iterator<Value>;

        template<bit_block_container_type OtherBits, storage OtherStore, window OtherWindow, class OtherDerived, std::size_t OtherE>
        friend class sequence_adaptor;

        friend class basic_reference<Value>;

        [[nodiscard]] constexpr basic_iterator(storage_ptr_t<Bits> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = Value;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = basic_reference<Value>;

        [[nodiscard]] basic_iterator() = default;

        // A mutable iterator converts to its const twin, as a container's iterator converts to its const_iterator.
        template<class MutableIterator>
                requires std::is_const_v<Bits> and std::same_as<MutableIterator, typename random_access<std::remove_const_t<Bits>>::iterator>
        [[nodiscard]] constexpr explicit(false) basic_iterator(MutableIterator other) noexcept // NOLINT(misc-explicit-constructor)
                : m_ptr(other.m_ptr)
                , m_idx(other.m_idx)
        {}

        [[nodiscard]] friend constexpr auto operator==(basic_iterator lhs, basic_iterator rhs) noexcept
                -> bool
        {
                assert(lhs.m_ptr == rhs.m_ptr);
                return lhs.m_idx == rhs.m_idx;
        }

        [[nodiscard]] friend constexpr auto operator<=>(basic_iterator lhs, basic_iterator rhs) noexcept
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
                -> basic_iterator&
        {
                ++m_idx;
                return *this;
        }

        constexpr auto operator--() noexcept
                -> basic_iterator&
        {
                --m_idx;
                return *this;
        }

        constexpr auto operator++(int) noexcept
                -> basic_iterator
        {
                auto nrv = *this;
                ++*this;
                return nrv;
        }

        constexpr auto operator--(int) noexcept
                -> basic_iterator
        {
                auto nrv = *this;
                --*this;
                return nrv;
        }

        constexpr auto operator+=(difference_type n) noexcept
                -> basic_iterator&
        {
                m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) + n);
                return *this;
        }

        constexpr auto operator-=(difference_type n) noexcept
                -> basic_iterator&
        {
                m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) - n);
                return *this;
        }

        [[nodiscard]] friend constexpr auto operator+(basic_iterator lhs, difference_type n) noexcept
                -> basic_iterator
        {
                auto nrv = lhs;
                nrv += n;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator+(difference_type n, basic_iterator rhs) noexcept
                -> basic_iterator
        {
                auto nrv = rhs;
                nrv += n;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator-(basic_iterator lhs, difference_type n) noexcept
                -> basic_iterator
        {
                auto nrv = lhs;
                nrv -= n;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator-(basic_iterator lhs, basic_iterator rhs) noexcept
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
        [[nodiscard]] friend constexpr auto iter_move(basic_iterator it) noexcept
                -> value_type
        {
                return *it;
        }

        friend constexpr auto iter_swap(basic_iterator x, basic_iterator y) noexcept
                -> void
                requires (not std::is_const_v<Bits>)
        {
                value_type const t = *x;
                *x                 = *y;
                *y                 = t;
        }
};

// A proxy bool spelled as [vector.bool] spells std::vector<bool>::reference; operator~ is std::bitset's.
template<class Bits>
template<class Value>
class random_access<Bits>::basic_reference
{
        // Value exists only to put the value type's namespaces among the associated ones: it is no second axis.
        static_assert(std::same_as<Value, random_access::value_type>);

public:
        using value_type     = Value;
        using iterator       = basic_iterator<Value>;
        using enclosing_type = random_access;

private:
        storage_ptr_t<Bits> m_ptr;
        std::size_t m_idx;

        // Writable where Bits is not const: a const storage has no assign to reach.
        static constexpr bool is_writable = not std::is_const_v<Bits> and requires (Bits& c, std::size_t n, value_type value) { c.assign(n, value); };

        template<bit_block_container_type OtherBits, storage OtherStore, window OtherWindow, class OtherDerived, std::size_t OtherE>
        friend class sequence_adaptor;

        friend class basic_iterator<Value>;

        [[nodiscard]] constexpr basic_reference(storage_ptr_t<Bits> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        // Said out loud: the assignments below are user-provided, which deprecates the implicit copy constructor.
        basic_reference(basic_reference const&) = default;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return {m_ptr, m_idx};
        }

        // The one conversion, as std::vector<bool>::reference has: comparisons are the built-in ones through it.
        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return m_ptr->test(m_idx);
        }

        // const-qualified and returning a const reference, the proxy shape P2321R2 gave std::vector<bool>::reference.
        constexpr auto operator=(value_type value) const noexcept // NOLINT(misc-unconventional-assign-operator)
                -> basic_reference const&
                requires is_writable
        {
                m_ptr->assign(m_idx, value);
                return *this;
        }

        // Assigns the bit, not the proxy: rebinding would break the swaps below.
        constexpr auto operator=(basic_reference const& other) const noexcept // NOLINT(misc-unconventional-assign-operator,bugprone-unhandled-self-assignment)
                -> basic_reference const&
                requires is_writable
        {
                return *this = static_cast<value_type>(other);
        }

        // [vector.bool] has required it of the proxy since C++98, where the const-qualified assignment is C++23.
        constexpr auto flip() const noexcept
                -> void
                requires is_writable
        {
                m_ptr->assign(m_idx, not static_cast<value_type>(*this));
        }

        // The pre-ranges spelling of iter_swap, for std::swap and the algorithms still built on it.
        friend constexpr auto swap(basic_reference x, basic_reference y) noexcept -> void
                requires is_writable
        {
                value_type const t = x;
                x                  = y;
                y                  = t;
        }

        friend constexpr auto swap(basic_reference x, value_type& y) noexcept -> void
                requires is_writable
        {
                value_type const t = x;
                x                  = y;
                y                  = t;
        }

        friend constexpr auto swap(value_type& x, basic_reference y) noexcept -> void
                requires is_writable
        {
                value_type const t = x;
                x                  = y;
                y                  = t;
        }

        // What this proxy prints as, said once: our std::formatter calls it unqualified, and fmt finds it by ADL.
        [[nodiscard]] friend constexpr auto format_as(basic_reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

} // namespace xstd::bits::detail

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

// std::format over the containers, which needs nothing said about the containers themselves.
template<xstd::bits::detail::random_access_reference R, class CharT>
struct formatter<R, CharT> : formatter<bool, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(R ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend.
                return formatter<bool, CharT>::format(format_as(ref), ctx);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
