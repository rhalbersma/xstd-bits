//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
#define XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP

#include <xstd/bits/bit_key_traits.hpp>             // bit_key_traits
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container_type
#include <xstd/bits/detail/ownership.hpp>           // storage
#include <xstd/bits/detail/storage_ptr.hpp>         // storage_ptr_t
#include <xstd/bits/detail/zero_width.hpp>          // zero_width
#include <cassert>                                  // assert
#include <concepts>                                 // same_as
#include <cstddef>                                  // ptrdiff_t, size_t
#include <format>                                   // formatter
#include <iterator>                                 // bidirectional_iterator_tag
#include <type_traits>                              // remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through the storage alone.
namespace xstd::bits::detail {

// The order a set reading walks its positions in, which is all a comparator can choose.
enum struct direction : bool
{
        ascending,
        descending,
};

// Member templates of a class template: only Key, their own argument, puts its namespaces in ADL's associated set.
template<class Bits, class KeyTraits, direction Direction>
struct bidirectional
{
        template<class Key>
        class iterator;

        template<class Key>
        class reference;
};

template<class Bits, class Key = std::size_t, class KeyTraits = bit_key_traits<Key>, direction Direction = direction::ascending>
using bidirectional_bit_iterator = bidirectional<Bits, KeyTraits, Direction>::template iterator<Key>;

template<class Bits, class Key = std::size_t, class KeyTraits = bit_key_traits<Key>, direction Direction = direction::ascending>
using bidirectional_bit_reference = bidirectional<Bits, KeyTraits, Direction>::template reference<Key>;

template<class>
inline constexpr bool is_bidirectional = false;

template<class Bits, class KeyTraits, direction Direction>
inline constexpr bool is_bidirectional<bidirectional<Bits, KeyTraits, Direction>> = true;

// Recognized through the enclosing class it names, since no deduction reaches Bits through a nested class.
template<class R>
concept bidirectional_reference = is_bidirectional<typename R::enclosing_type> and std::same_as<R, typename R::enclosing_type::template reference<typename R::value_type>>;

template<bit_block_container_type Bits, storage Store, class Derived, class Key, class KeyTraits, class Compare>
class set_adaptor;

// A position in the set reading, read-only whatever Bits' qualification: a key is nothing to write through.
template<class Bits, class KeyTraits, direction Direction>
template<class Key>
class bidirectional<Bits, KeyTraits, Direction>::iterator
{
        using bits_type = std::remove_const_t<Bits>;

        storage_ptr_t<bits_type const> m_ptr{};
        std::size_t m_idx{};

        template<bit_block_container_type OtherBits, storage OtherStore, class OtherDerived, class OtherKey, class OtherKeyTraits, class OtherCompare>
        friend class set_adaptor;

        friend class bidirectional::reference<Key>;

        [[nodiscard]] constexpr iterator(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = Key;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = bidirectional::reference<Key>;

        [[nodiscard]] iterator() = default;

        // A zero width has one position, so every iterator over it is the same one and every loop stops early.
        [[nodiscard]] friend constexpr auto operator==(iterator lhs, iterator rhs) noexcept
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
                -> iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        assert(m_idx < m_ptr->size());
                        if constexpr (Direction == direction::ascending) {
                                m_idx = m_ptr->exclusive_find_next(m_idx);
                        } else {
                                m_idx = m_ptr->total_find_prev(m_idx);
                        }
                }
                return *this;
        }

        // Descending, the end is size() as well, so stepping back from it is a step up to the lowest position.
        constexpr auto operator--() noexcept
                -> iterator&
        {
                assert(m_ptr != nullptr);
                if constexpr (not zero_width<Bits>) {
                        if constexpr (Direction == direction::ascending) {
                                assert(m_ptr->find_first() < m_idx);
                                m_idx = m_ptr->exclusive_find_prev(m_idx);
                        } else if (m_idx == m_ptr->size()) {
                                assert(m_ptr->find_first() < m_ptr->size());
                                m_idx = m_ptr->find_first();
                        } else {
                                assert(m_ptr->exclusive_find_next(m_idx) < m_ptr->size());
                                m_idx = m_ptr->exclusive_find_next(m_idx);
                        }
                }
                return *this;
        }

        constexpr auto operator++(int) noexcept
                -> iterator
        {
                auto nrv = *this;
                ++*this;
                return nrv;
        }

        constexpr auto operator--(int) noexcept
                -> iterator
        {
                auto nrv = *this;
                --*this;
                return nrv;
        }
};

// The key at a position, arriving by conversion through KeyTraits; & hands the iterator back, so the pair round-trips.
template<class Bits, class KeyTraits, direction Direction>
template<class Key>
class bidirectional<Bits, KeyTraits, Direction>::reference
{
        using bits_type = std::remove_const_t<Bits>;

        storage_ptr_t<bits_type const> m_ptr;
        std::size_t m_idx;

        template<bit_block_container_type OtherBits, storage OtherStore, class OtherDerived, class OtherKey, class OtherKeyTraits, class OtherCompare>
        friend class set_adaptor;

        friend class bidirectional::iterator<Key>;

        [[nodiscard]] constexpr reference(storage_ptr_t<bits_type const> ptr, std::size_t idx) noexcept
                : m_ptr(ptr)
                , m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

public:
        using value_type     = Key;
        using iterator       = bidirectional::iterator<Key>;
        using enclosing_type = bidirectional;

        // A value, not a handle to rebind: trivially copyable, never assignable, as a reference to a key is.
        reference(reference const&)                    = default;
        auto operator=(reference const&) -> reference& = delete;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return {m_ptr, m_idx};
        }

        // The one conversion: comparisons are the key's own through it.
        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return KeyTraits::from_index(m_idx);
        }

        // What this proxy prints as, said once: our std::formatter calls it unqualified, and fmt finds it by ADL.
        [[nodiscard]] friend constexpr auto format_as(reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

} // namespace xstd::bits::detail

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

// std::format over the containers, which prints the key as the key's own formatter does.
template<xstd::bits::detail::bidirectional_reference R, class CharT>
struct formatter<R, CharT> : formatter<typename R::value_type, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(R ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend.
                return formatter<typename R::value_type, CharT>::format(format_as(ref), ctx);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_DETAIL_BIDIRECTIONAL_HPP
