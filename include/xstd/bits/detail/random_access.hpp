//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
#define XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP

#include <xstd/bits/bit_traits.hpp>       // bit_storage, bit_traits
#include <xstd/ints/concepts/integer.hpp> // integer
#include <cassert>                        // assert
#include <compare>                        // strong_ordering
#include <concepts>                       // same_as
#include <cstddef>                        // ptrdiff_t, size_t
#include <format>                         // formatter
#include <iterator>                       // random_access_iterator_tag
#include <type_traits>                    // is_class_v, is_const_v, is_convertible_v, is_nothrow_constructible_v, remove_const_t

// The iterator is the primitive: a pointer and a position, reaching the bits through Traits alone. [design.md#the-iterator-is-the-primitive]
// The sequence reading's pair, named after the category its iterator models; the set reading's is bidirectional.hpp.
namespace xstd::detail::bits {

template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class random_access_bit_iterator;
template<class Bits, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>> class random_access_bit_reference;

// A position in the sequence reading; const Bits is the const iterator, the old IsConst bool folded into the type.
template<class Bits, bit_storage<Bits> Traits>
class random_access_bit_iterator
{
        Bits* m_ptr{};
        std::size_t m_idx{};

        // The const twin, whose conversion below reads these members; naming itself where Bits is already const.
        friend class random_access_bit_iterator<Bits const, Traits>;

public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = bool;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = random_access_bit_reference<Bits, Traits>;

        [[nodiscard]] constexpr random_access_bit_iterator() noexcept = default;

        [[nodiscard]] constexpr random_access_bit_iterator(Bits* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // A mutable iterator converts to its const twin, as a container's iterator converts to its const_iterator.
        template<class Mutable>
                requires std::is_const_v<Bits> and std::same_as<Mutable const, Bits>
        [[nodiscard]] constexpr explicit(false) random_access_bit_iterator(random_access_bit_iterator<Mutable, Traits> other) noexcept  // NOLINT(misc-explicit-constructor)
        :
                m_ptr(other.m_ptr),
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

        [[nodiscard]] constexpr auto operator*() const noexcept
                -> reference
        {
                assert(m_ptr != nullptr);
                return { m_ptr, m_idx };
        }

        constexpr auto operator++() noexcept -> random_access_bit_iterator& { ++m_idx; return *this; }
        constexpr auto operator--() noexcept -> random_access_bit_iterator& { --m_idx; return *this; }

        constexpr auto operator++(int) noexcept -> random_access_bit_iterator { auto nrv = *this; ++*this; return nrv; }
        constexpr auto operator--(int) noexcept -> random_access_bit_iterator { auto nrv = *this; --*this; return nrv; }

        constexpr auto operator+=(difference_type n) noexcept -> random_access_bit_iterator& { m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) + n); return *this; }
        constexpr auto operator-=(difference_type n) noexcept -> random_access_bit_iterator& { m_idx = static_cast<std::size_t>(static_cast<difference_type>(m_idx) - n); return *this; }

        [[nodiscard]] friend constexpr auto operator+(random_access_bit_iterator lhs, difference_type n) noexcept -> random_access_bit_iterator { auto nrv = lhs; nrv += n; return nrv; }
        [[nodiscard]] friend constexpr auto operator+(difference_type n, random_access_bit_iterator rhs) noexcept -> random_access_bit_iterator { auto nrv = rhs; nrv += n; return nrv; }
        [[nodiscard]] friend constexpr auto operator-(random_access_bit_iterator lhs, difference_type n) noexcept -> random_access_bit_iterator { auto nrv = lhs; nrv -= n; return nrv; }

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

        // The one ADL exception: std::ranges' own protocol, which is how sort and swap_ranges reach a proxy. [design.md#the-one-adl-exception]
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

// A proxy bool assigning back through the trait; std::vector<bool>::reference is the precedent for the const-qualified assignment, and nothing more is borrowed: no flip, no ~.
template<class Bits, bit_storage<Bits> Traits>
class random_access_bit_reference
{
        Bits* m_ptr;
        std::size_t m_idx;

        // Writable where Bits is not const and the trait declares unchecked_assign; a trait with only the required entries reads only.
        static constexpr bool is_writable = not std::is_const_v<Bits> and requires (Bits& c, std::size_t n, bool value) { Traits::unchecked_assign(c, n, value); };

public:
        using value_type = bool;
        using iterator   = random_access_bit_iterator<Bits, Traits>;

        [[nodiscard]] constexpr random_access_bit_reference(Bits* ptr, std::size_t idx) noexcept
        :
                m_ptr(ptr),
                m_idx(idx)
        {
                assert(m_ptr != nullptr);
        }

        // Said out loud, because the assignments below are user-provided and that deprecates the implicit copy
        // constructor: a copy duplicates the handle, where an assignment writes through it. The two do different
        // things here, which is exactly why the compiler stops guessing. [design.md#the-proxy-copies-the-handle]
        constexpr random_access_bit_reference(random_access_bit_reference const&) noexcept = default;

        [[nodiscard]] constexpr auto operator&() const noexcept
                -> iterator
        {
                return { m_ptr, m_idx };
        }

        [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept  // NOLINT(misc-explicit-constructor)
        {
                return Traits::at(*m_ptr, m_idx);
        }

        // Not to an integer, though, however class-shaped it is. A Block that is an integer CLASS -- MSVC's
        // std::_Unsigned128, absl::uint128, boost::int128::uint128 -- is constructible from bool and brings a
        // full set of operators, so without this exclusion every operator on a proxy has two equally good
        // readings: convert both sides to bool, or convert both sides to the Block. That ambiguity is not
        // confined to ==; it takes ! and every other operator the integer class declares with it, which is why
        // the exclusion belongs here on the conversion rather than on each operator in turn. The proxy stands
        // for one bit, and a bit is not an integer. [design.md#uint128-support]
        template<class T>
        [[nodiscard]] constexpr explicit(false) operator T() const noexcept(std::is_nothrow_constructible_v<T, value_type>)  // NOLINT(misc-explicit-constructor)
                requires std::is_class_v<T> and std::is_convertible_v<value_type, T> and (not xstd::integer<T>)
        {
                return Traits::at(*m_ptr, m_idx);
        }

        // Exact matches, so a comparison never reaches for a conversion. The exclusion above stops the proxy
        // becoming the Block, but it cannot stop the Block's own operators from being CANDIDATES: the proxy
        // names its Block among its template arguments, so the Block's namespace is an associated one and ADL
        // brings in whatever templated comparisons it declares -- Boost.Int128 declares exactly such a set.
        // These two are exact in both operands and win outright, which is what keeps the proxy
        // equality_comparable and std::ranges::equal working over it.
        //
        // Only here, and not on bidirectional.hpp's set proxy, which needs none of this: its value_type is a
        // position rather than a bit, a set over an integer-class Block already worked, and giving it the same
        // pair broke comparing two DIFFERENT instantiations of it -- which is how a view named on the adaptor
        // is compared against one deduced from the storage, at a Block as ordinary as uint64_t.
        // [design.md#uint128-support]
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
        constexpr auto operator=(bool value) const noexcept  // NOLINT(misc-unconventional-assign-operator)
                -> random_access_bit_reference const&
                requires is_writable
        {
                Traits::unchecked_assign(*m_ptr, m_idx, value);
                return *this;
        }

        // Assigns the bit, not the proxy: rebinding would break the swaps below. [design.md#clang-tidy-false-positives]
        constexpr auto operator=(random_access_bit_reference const& other) const noexcept  // NOLINT(misc-unconventional-assign-operator,bugprone-unhandled-self-assignment)
                -> random_access_bit_reference const&
                requires is_writable
        {
                return *this = static_cast<bool>(other);
        }

        // The pre-ranges spelling of iter_swap, for std::swap and the algorithms still built on it. [design.md#the-one-adl-exception]
        friend constexpr auto swap(random_access_bit_reference x, random_access_bit_reference y) noexcept -> void requires is_writable { bool const t = x; x = y; y = t; }
        friend constexpr auto swap(random_access_bit_reference x, bool& y)                 noexcept -> void requires is_writable { bool const t = x; x = y; y = t; }
        friend constexpr auto swap(bool& x, random_access_bit_reference y)                 noexcept -> void requires is_writable { bool const t = x; x = y; y = t; }

        [[nodiscard]] friend constexpr auto format_as(random_access_bit_reference ref) noexcept
                -> value_type
        {
                return ref;
        }
};

}       // namespace xstd::detail::bits


// std::format over the containers, which needs nothing said about the containers themselves.
// [design.md#formatting-the-proxies]
//
// Every owner and view here is already a range, so [format.range.formatter] would format it -- except that the
// range formatter requires formattable<range_reference_t<R>>, and a reference of ours is a proxy. So the proxy
// is what gets a formatter, and every container over it follows.
//
// It defers to format_as, the hook fmt already calls, so the value this proxy prints as is defined once and both
// libraries read it from there. Deriving from the underlying formatter rather than writing parse() is what keeps
// the whole format spec: a width, a fill, {:#x} on a position and {:d} on a bool, and the nested spec a range
// formatter forwards ({::#x}) reaching them.
//
// [namespace.std]/2 allows a specialization of a standard library template for a program-defined type, which is
// what this is and all it is. clang-tidy 22 and 23 read the qualified definition as modifying namespace std
// anyway; 24 no longer does. [design.md#clang-tidy-false-positives]
template<class Bits, class Traits, class CharT>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct std::formatter<xstd::detail::bits::random_access_bit_reference<Bits, Traits>, CharT>
:
        std::formatter<bool, CharT>
{
        template<class Context>
        [[nodiscard]] constexpr auto format(xstd::detail::bits::random_access_bit_reference<Bits, Traits> ref, Context& ctx) const
        {
                // Unqualified, so ADL finds the proxy's own hidden friend. [design.md#the-one-adl-exception]
                return std::formatter<bool, CharT>::format(format_as(ref), ctx);
        }
};

#endif  // XSTD_BITS_DETAIL_RANDOM_ACCESS_HPP
