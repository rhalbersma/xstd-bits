//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
#define XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP

#include <xstd/bits/bit_key_traits.hpp>       // bit_key_traits
#include <xstd/bits/detail/bit_container.hpp> // bit_container_type
#include <xstd/bits/detail/ownership.hpp>     // storage
#include <xstd/bits/detail/storage_ptr.hpp>   // storage_ptr_t
#include <xstd/bits/detail/zero_width.hpp>    // zero_width
#include <cassert>                            // assert
#include <cstddef>                            // ptrdiff_t, size_t
#include <format>                             // formatter
#include <iterator>                           // bidirectional_iterator_tag
#include <type_traits>                        // is_class_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through the storage alone.
namespace xstd::bits::detail {

template<class Bits, class Key = std::size_t, class KeyTraits = bit_key_traits<Key>>
class bidirectional_bit_iterator;

template<class Bits, class Key = std::size_t, class KeyTraits = bit_key_traits<Key>>
class bidirectional_bit_reference;

template<bit_container_type Bits, storage Store, class Derived, class Key, class KeyTraits>
class set_adaptor;

// A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through.
template<class Bits, class Key, class KeyTraits>
class bidirectional_bit_iterator
{
        using bits_type = std::remove_const_t<Bits>;

        storage_ptr_t<bits_type const> m_ptr{};
        std::size_t m_idx{};

        template<bit_container_type B, storage S, class D, class K, class T>
        friend class set_adaptor;
        friend class bidirectional_bit_reference<Bits, Key, KeyTraits>;

        [[nodiscard]] constexpr bidirectional_bit_iterator(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Key;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = bidirectional_bit_reference<Bits, Key, KeyTraits>;

        [[nodiscard]] bidirectional_bit_iterator() noexcept = default;

        // A zero width has one position, so every iterator over it is the same one and every loop stops early.
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
                return {m_ptr, m_idx};
        }

        // Both steps guarded at a zero width: the exclusive scans take a position it has none to give.
        constexpr auto operator++() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        assert(m_idx < m_ptr->size());
                        m_idx = m_ptr->exclusive_find_next(m_idx);
                }
                return *this;
        }

        constexpr auto operator--() noexcept
                -> bidirectional_bit_iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        assert(m_ptr->find_first() < m_idx);
                        m_idx = m_ptr->exclusive_find_prev(m_idx);
                }
                return *this;
        }

        constexpr auto operator++(int) noexcept
                -> bidirectional_bit_iterator
        {
                auto nrv = *this;
                ++*this;
                return nrv;
        }

        constexpr auto operator--(int) noexcept
                -> bidirectional_bit_iterator
        {
                auto nrv = *this;
                --*this;
                return nrv;
        }
};

// The key at a position, arriving by conversion through KeyTraits; & hands the iterator back, so the pair round-trips.
template<class Bits, class Key, class KeyTraits>
class bidirectional_bit_reference
{
        using bits_type = std::remove_const_t<Bits>;

        storage_ptr_t<bits_type const> m_ptr;
        std::size_t m_idx;

        template<bit_container_type B, storage S, class D, class K, class T>
        friend class set_adaptor;
        friend class bidirectional_bit_iterator<Bits, Key, KeyTraits>;

        [[nodiscard]] constexpr bidirectional_bit_reference(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        using value_type = Key;
        using iterator = bidirectional_bit_iterator<Bits, Key, KeyTraits>;

        // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is.
        bidirectional_bit_reference(bidirectional_bit_reference const&) noexcept = default;
        auto operator=(bidirectional_bit_reference const&) -> bidirectional_bit_reference& = delete;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return {m_ptr, m_idx};
        }

        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return KeyTraits::from_index(m_idx);
        }

        // As a held key would, the key initializes any class implicitly constructible from it, through KeyTraits.
        template<class T>
        [[nodiscard]] constexpr explicit(false) operator T() const noexcept(std::is_nothrow_constructible_v<T, value_type>) // NOLINT(misc-explicit-constructor)
                requires std::is_class_v<T> and std::is_convertible_v<value_type, T>
        {
                return KeyTraits::from_index(m_idx);
        }

        // What this proxy prints as, said once: our std::formatter calls it unqualified, and fmt finds it by ADL.
        [[nodiscard]] friend constexpr auto format_as(bidirectional_bit_reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

} // namespace xstd::bits::detail

// std::format over the containers, which prints the key as the key's own formatter does.
template<class Bits, class Key, class KeyTraits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::bits::detail::bidirectional_bit_reference<Bits, Key, KeyTraits>, CharT> : std::formatter<Key, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::bits::detail::bidirectional_bit_reference<Bits, Key, KeyTraits> ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend.
                return std::formatter<Key, CharT>::format(format_as(ref), ctx);
        }
};

#endif // XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
