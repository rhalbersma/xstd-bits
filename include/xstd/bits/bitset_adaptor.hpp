//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_ADAPTOR_HPP
#define XSTD_BITS_BITSET_ADAPTOR_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <boost/hash2/hash_append.hpp> // hash_append_tag
#include <xstd/bits/bit_traits.hpp>    // bit_storage, bit_traits, static_bit_extent, zero_width
#include <xstd/bits/detail/hash.hpp>   // hash_append_bits, std_hash
#include <xstd/bits/ownership.hpp>     // owned_storage, ownership
#include <algorithm>                   // min
#include <cassert>                     // assert
#include <concepts>                    // convertible_to, regular, same_as
#include <cstddef>                     // size_t
#include <format>                      // format
#include <functional>                  // hash
#include <ios>                         // ios_base
#include <iosfwd>                      // basic_istream, basic_ostream
#include <iterator>                    // input_iterator
#include <limits>                      // numeric_limits
#include <locale>                      // ctype, use_facet
#include <memory>                      // allocator
#include <ranges>                      // iota
#include <source_location>             // source_location
#include <stdexcept>                   // invalid_argument, out_of_range, overflow_error
#include <string>                      // basic_string, char_traits
#include <string_view>                 // basic_string_view
#include <utility>                     // as_const

namespace xstd {

// The bitset vocabulary a storage speaks natively, one line each in the wrapper; a backend missing a member fails here, at the class.
// The shifts stay in although bit_traits carries their contracts: without them a shiftless backend would fail inside an instantiation. [design.md#the-idempotent-wrapper]
template<class Bits>
concept has_bitops =
        std::regular<Bits> and
        requires (Bits& b, Bits const& c, std::size_t n)
        {
                { b &= c    } -> std::same_as<Bits&>;
                { b |= c    } -> std::same_as<Bits&>;
                { b ^= c    } -> std::same_as<Bits&>;
                { b <<= n   } -> std::same_as<Bits&>;
                { b >>= n   } -> std::same_as<Bits&>;
                { b.set()   } -> std::same_as<Bits&>;
                { b.reset() } -> std::same_as<Bits&>;
                { b.flip()  } -> std::same_as<Bits&>;
                { c.all()   } -> std::same_as<bool>;
                { c.any()   } -> std::same_as<bool>;
                { c.none()  } -> std::same_as<bool>;
                { c.count() } -> std::convertible_to<std::size_t>;
                { c.size()  } -> std::convertible_to<std::size_t>;
        }
;

// [template.bitset] over any Bits that speaks the vocabulary: what Bits has is forwarded, what it lacks is added through Traits. [design.md#the-idempotent-wrapper]
template<has_bitops Bits, bit_storage<Bits> Traits = bit_traits<Bits>>
class bitset_adaptor
{
        // Two counterparts, one wrapper: std::bitset at a static width, boost::dynamic_bitset at a run-time one. [design.md#the-idempotent-wrapper]
        static constexpr bool has_static_width = static_bit_extent<Traits, Bits>;

        // No iteration and no <=> here by design, because neither counterpart has them: the two views refer into the storage instead. [design.md#views-over-owners]
        Bits m_bits{};

        template<class B, ownership O, bit_storage<B> T>         friend class set_adaptor;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class sequence_adaptor;

        // The value through the door, so a wrapper over std::bitset hashes on every library, whether or not its bits can be read by block. [design.md#the-hashing-invariant]
        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bitset_adaptor const* v) noexcept
        {
                detail::bits::hash_append_bits<Traits>(h, f, v->m_bits);
        }

public:
        // boost's typedef, which the harness keys a run-time width on; std::bitset has none, and a typedef changes no answer.
        using size_type = std::size_t;

        // [bitset.refs], reaching the bits only through the unchecked way in; its own class, with the flip and ~ the sequence proxy lacks. [design.md#unchecked-writes-in-views]
        class reference
        {
                // A pointer, not a reference, so the copy constructor stays defaulted as [bitset.refs] declares it.
                bitset_adaptor* m_ptr{};
                std::size_t m_idx{};

                friend bitset_adaptor;

                [[nodiscard]] constexpr reference(bitset_adaptor& c, std::size_t idx) noexcept
                :
                        m_ptr(&c),
                        m_idx(idx)
                {}

        public:
                constexpr reference(reference const& x) noexcept = default;
                constexpr ~reference() = default;

                constexpr auto operator=(bool x) noexcept -> reference&
                {
                        std::as_const(*this) = x;
                        return *this;
                }

                // Assigns the bit, not the proxy: rebinding would break the swap below. [design.md#clang-tidy-false-positives]
                constexpr auto operator=(reference const& x) noexcept -> reference&  // NOLINT(bugprone-unhandled-self-assignment)
                {
                        std::as_const(*this) = static_cast<bool>(x);
                        return *this;
                }

                // A proxy reference assigns through a const proxy, the shape the standard gives vector<bool>::reference.
                constexpr auto operator=(bool x) const noexcept -> reference const&  // NOLINT(misc-unconventional-assign-operator)
                {
                        Traits::unchecked_assign(m_ptr->m_bits, m_idx, x);
                        return *this;
                }

                [[nodiscard]] constexpr explicit(false) operator bool() const noexcept  // NOLINT(misc-explicit-constructor)
                {
                        return Traits::at(m_ptr->m_bits, m_idx);
                }

                [[nodiscard]] constexpr auto operator~() const noexcept -> bool
                {
                        return not Traits::at(m_ptr->m_bits, m_idx);
                }

                friend constexpr void swap(reference x, reference y) noexcept { bool const t = x; x = y; y = t; }
                friend constexpr void swap(reference x,     bool& y) noexcept { bool const t = x; x = y; y = t; }
                friend constexpr void swap(    bool& x, reference y) noexcept { bool const t = x; x = y; y = t; }

                constexpr auto flip() noexcept
                        -> reference&
                {
                        Traits::unchecked_assign(m_ptr->m_bits, m_idx, not Traits::at(m_ptr->m_bits, m_idx));
                        return *this;
                }
        };

        // boost's sentinel, for the two searches a run-time width answers with it.
        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

        // Constructors                                            [bitset.cons]
        [[nodiscard]] constexpr bitset_adaptor() noexcept = default;

        // [bitset.cons]/2: the low bits of val, as many as the width admits; boost takes the width first and the value second.
        [[nodiscard]] constexpr explicit(false) bitset_adaptor(unsigned long long val) noexcept  // NOLINT(misc-explicit-constructor)
                requires has_static_width
        {
                from_ullong(val);
        }

        [[nodiscard]] constexpr explicit bitset_adaptor(std::size_t num_bits, unsigned long long val = 0ULL)
                requires (not has_static_width)
        :
                m_bits(num_bits)
        {
                from_ullong(val);
        }

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                std::basic_string<charT, traits, Allocator> const& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one  = static_cast<charT>('1')
        )
        :
                bitset_adaptor(std::basic_string_view<charT, traits>(str), pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                std::basic_string_view<charT, traits> str,
                std::basic_string_view<charT, traits>::size_type pos = 0,
                std::basic_string_view<charT, traits>::size_type n = std::basic_string_view<charT, traits>::npos,
                charT zero = static_cast<charT>('0'),
                charT one  = static_cast<charT>('1')
        )
        {
                if (pos > str.size()) {
                        throw out_of_range(pos);
                }
                auto const rlen = std::ranges::min(n, str.size() - pos);
                // A run-time width is the characters read, as boost's is when no width is given.
                if constexpr (not has_static_width) {
                        m_bits.resize(rlen);
                }
                auto const M = std::ranges::min(size(), rlen);
                for (auto const i : std::views::iota(0UZ, M)) {
                        auto const ch = str[pos + M - 1 - i];
                        if (traits::eq(ch, zero)) {
                                continue;
                        }
                        if (traits::eq(ch, one)) {
                                Traits::unchecked_assign(m_bits, i, true);
                        } else {
                                throw invalid_argument(ch, zero, one);
                        }
                }
        }

        template<class charT>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                charT const* str,
                std::basic_string_view<charT>::size_type n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one  = static_cast<charT>('1')
        )
        :
                bitset_adaptor(n == std::basic_string_view<charT>::npos ? std::basic_string_view<charT>(str) : std::basic_string_view<charT>(str, n), 0, n, zero, one)
        {}

        // Members                                              [bitset.members]
        constexpr auto operator&=(bitset_adaptor const& rhs) noexcept -> bitset_adaptor& { m_bits &= rhs.m_bits; return *this; }
        constexpr auto operator|=(bitset_adaptor const& rhs) noexcept -> bitset_adaptor& { m_bits |= rhs.m_bits; return *this; }
        constexpr auto operator^=(bitset_adaptor const& rhs) noexcept -> bitset_adaptor& { m_bits ^= rhs.m_bits; return *this; }

        // Same spelling, two contracts: the trait's checked entry is total and forwarded as is; the storage's own shift is the unchecked one, guarded here. [design.md#checked-and-unchecked]
        constexpr auto operator<<=(std::size_t pos) noexcept
                -> bitset_adaptor&
        {
                if constexpr (requires { Traits::checked_shift_left(m_bits, pos); }) {
                        Traits::checked_shift_left(m_bits, pos);
                } else if (pos < size()) {
                        m_bits <<= pos;
                } else {
                        m_bits.reset();
                }
                return *this;
        }

        constexpr auto operator>>=(std::size_t pos) noexcept
                -> bitset_adaptor&
        {
                if constexpr (requires { Traits::checked_shift_right(m_bits, pos); }) {
                        Traits::checked_shift_right(m_bits, pos);
                } else if (pos < size()) {
                        m_bits >>= pos;
                } else {
                        m_bits.reset();
                }
                return *this;
        }

        [[nodiscard]] constexpr auto operator<<(std::size_t pos) const noexcept(has_static_width) -> bitset_adaptor { auto nrv = *this; nrv <<= pos; return nrv; }
        [[nodiscard]] constexpr auto operator>>(std::size_t pos) const noexcept(has_static_width) -> bitset_adaptor { auto nrv = *this; nrv >>= pos; return nrv; }

        [[nodiscard]] constexpr auto operator~() const noexcept(has_static_width) -> bitset_adaptor { auto nrv = *this; nrv.flip(); return nrv; }

        constexpr auto set  () noexcept -> bitset_adaptor& { m_bits.set  (); return *this; }
        constexpr auto reset() noexcept -> bitset_adaptor& { m_bits.reset(); return *this; }
        constexpr auto flip () noexcept -> bitset_adaptor& { m_bits.flip (); return *this; }

        // Element access, both families: the trait's checked entry where the counterpart throws natively, else the guard and the unchecked write. [design.md#checked-and-unchecked]
        constexpr auto set(std::size_t pos, bool val = true)
                -> bitset_adaptor&
        {
                if constexpr (requires { Traits::checked_set(m_bits, pos, val); }) {
                        Traits::checked_set(m_bits, pos, val);
                } else if constexpr (has_static_width) {
                        if (pos < size()) {
                                Traits::unchecked_assign(m_bits, pos, val);
                        } else {
                                throw out_of_range(pos);
                        }
                } else {
                        // boost asserts, and so does its stand-in: the inconsistency with the static width is the counterparts' own. [design.md#checked-and-unchecked]
                        assert(pos < size());
                        Traits::unchecked_assign(m_bits, pos, val);
                }
                return *this;
        }

        constexpr auto reset(std::size_t pos)
                -> bitset_adaptor&
        {
                if constexpr (requires { Traits::checked_reset(m_bits, pos); }) {
                        Traits::checked_reset(m_bits, pos);
                } else if constexpr (has_static_width) {
                        if (pos < size()) {
                                Traits::unchecked_assign(m_bits, pos, false);
                        } else {
                                throw out_of_range(pos);
                        }
                } else {
                        assert(pos < size());
                        Traits::unchecked_assign(m_bits, pos, false);
                }
                return *this;
        }

        constexpr auto flip(std::size_t pos)
                -> bitset_adaptor&
        {
                if constexpr (requires { Traits::checked_flip(m_bits, pos); }) {
                        Traits::checked_flip(m_bits, pos);
                } else if constexpr (has_static_width) {
                        if (pos < size()) {
                                Traits::unchecked_assign(m_bits, pos, not Traits::at(m_bits, pos));
                        } else {
                                throw out_of_range(pos);
                        }
                } else {
                        assert(pos < size());
                        Traits::unchecked_assign(m_bits, pos, not Traits::at(m_bits, pos));
                }
                return *this;
        }

        // The const subscript is unchecked on every counterpart, so it is Traits::at unconditionally.
        [[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
                -> bool
        {
                return Traits::at(m_bits, pos);
        }

        [[nodiscard]] constexpr auto operator[](std::size_t pos) noexcept
                -> reference
        {
                return { *this, pos };
        }

        // [bitset.members]/34-37: the value the bits spell, or overflow_error where a set position lies beyond the word; boost's to_ulong is the same contract.
        [[nodiscard]] constexpr auto to_ulong()  const -> unsigned long      { return to_word<unsigned long>();      }
        [[nodiscard]] constexpr auto to_ullong() const -> unsigned long long { return to_word<unsigned long long>(); }

        template<
                class charT = char,
                class traits = std::char_traits<charT>,
                class Allocator = std::allocator<charT>
        >
        [[nodiscard]] constexpr auto to_string(charT zero = static_cast<charT>('0'), charT one = static_cast<charT>('1')) const
                -> std::basic_string<charT, traits, Allocator>
        {
                auto const N = size();
                // The finding is the zero-width instantiation, where empty is the answer. [design.md#clang-tidy-false-positives]
                auto str = std::basic_string<charT, traits, Allocator>(N, zero);  // NOLINT(bugprone-string-constructor)
                for (auto const i : std::views::iota(0UZ, N)) {
                        if (Traits::at(m_bits, N - 1 - i)) {
                                str[i] = one;
                        }
                }
                return str;
        }

        // observers
        [[nodiscard]] constexpr auto count() const noexcept -> std::size_t { return m_bits.count(); }
        [[nodiscard]] constexpr auto size()  const noexcept -> std::size_t { return m_bits.size();  }

        [[nodiscard]] constexpr auto operator==(bitset_adaptor const& rhs) const noexcept -> bool = default;

        [[nodiscard]] constexpr auto test(std::size_t pos) const
                -> bool
        {
                if constexpr (requires { { Traits::checked_test(m_bits, pos) } -> std::convertible_to<bool>; }) {
                        return Traits::checked_test(m_bits, pos);
                } else if constexpr (has_static_width) {
                        if (pos < size()) {
                                return Traits::at(m_bits, pos);
                        }
                        throw out_of_range(pos);
                } else {
                        assert(pos < size());
                        return Traits::at(m_bits, pos);
                }
        }

        [[nodiscard]] constexpr auto all()  const noexcept -> bool { return m_bits.all();  }
        [[nodiscard]] constexpr auto any()  const noexcept -> bool { return m_bits.any();  }
        [[nodiscard]] constexpr auto none() const noexcept -> bool { return m_bits.none(); }

        // The set vocabulary boost has and std::bitset has not, forwarded exactly where the counterpart is boost: a run-time width, whose storage spells them alike. [design.md#the-idempotent-wrapper]
        constexpr auto operator-=(bitset_adaptor const& rhs) noexcept
                -> bitset_adaptor&
                requires (not has_static_width) and requires (Bits& b, Bits const& c) { b -= c; }
        {
                m_bits -= rhs.m_bits;
                return *this;
        }

        [[nodiscard]] constexpr auto is_subset_of(bitset_adaptor const& rhs) const noexcept
                -> bool
                requires (not has_static_width) and requires (Bits const& c) { c.is_subset_of(c); }
        {
                return m_bits.is_subset_of(rhs.m_bits);
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(bitset_adaptor const& rhs) const noexcept
                -> bool
                requires (not has_static_width) and requires (Bits const& c) { c.is_proper_subset_of(c); }
        {
                return m_bits.is_proper_subset_of(rhs.m_bits);
        }

        [[nodiscard]] constexpr auto intersects(bitset_adaptor const& rhs) const noexcept
                -> bool
                requires (not has_static_width) and requires (Bits const& c) { c.intersects(c); }
        {
                return m_bits.intersects(rhs.m_bits);
        }

        // boost's two searches, npos where the trait's total answer is the width.
        [[nodiscard]] constexpr auto find_first() const noexcept
                -> std::size_t
                requires (not has_static_width)
        {
                auto const n = detail::bits::find_first<Traits>(m_bits);
                return n == size() ? npos : n;
        }

        [[nodiscard]] constexpr auto find_next(std::size_t pos) const noexcept
                -> std::size_t
                requires (not has_static_width)
        {
                auto const n = detail::bits::find_next<Traits>(m_bits, pos);
                return n == size() ? npos : n;
        }

        // Growth, boost's members, on storage that spells them alike: detected on the storage rather than reconciled by the trait. [design.md#growth]
        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
                requires (not has_static_width)
        {
                return size() == 0UZ;
        }

        constexpr void resize(std::size_t num_bits, bool value = false)
                requires requires (Bits& b) { b.resize(num_bits, value); }
        {
                m_bits.resize(num_bits, value);
        }

        constexpr void clear()
                requires requires (Bits& b) { b.clear(); }
        {
                m_bits.clear();
        }

        constexpr void push_back(bool bit)
                requires requires (Bits& b) { b.push_back(bit); }
        {
                m_bits.push_back(bit);
        }

        constexpr void pop_back()
                requires requires (Bits& b) { b.pop_back(); }
        {
                m_bits.pop_back();
        }

        template<class Block>
        constexpr void append(Block value)
                requires requires (Bits& b) { b.append(value); }
        {
                m_bits.append(value);
        }

        template<std::input_iterator I>
        constexpr void append(I first, I last)
                requires requires (Bits& b) { b.append(first, last); }
        {
                m_bits.append(first, last);
        }

        constexpr void reserve(std::size_t num_bits)
                requires requires (Bits& b) { b.reserve(num_bits); }
        {
                m_bits.reserve(num_bits);
        }

        [[nodiscard]] constexpr auto capacity() const noexcept
                -> std::size_t
                requires requires (Bits const& c) { c.capacity(); }
        {
                return m_bits.capacity();
        }

        constexpr void shrink_to_fit()
                requires requires (Bits& b) { b.shrink_to_fit(); }
        {
                m_bits.shrink_to_fit();
        }

private:
        constexpr void from_ullong(unsigned long long val) noexcept
        {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<unsigned long long>::digits);
                auto const M = std::ranges::min(size(), digits);
                for (auto const i : std::views::iota(0UZ, M)) {
                        if (((val >> i) & 1ULL) != 0ULL) {
                                Traits::unchecked_assign(m_bits, i, true);
                        }
                }
        }

        // One position at a time over at most a word's worth, the overflow asked of the trait's search above the word.
        template<class Word>
        [[nodiscard]] constexpr auto to_word() const
                -> Word
        {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Word>::digits);
                if (size() > digits and detail::bits::find_next<Traits>(m_bits, digits - 1UZ) != size()) {
                        throw overflow_error();
                }
                auto const M = std::ranges::min(size(), digits);
                auto word = Word{0};
                for (auto const i : std::views::iota(0UZ, M)) {
                        if (Traits::at(m_bits, i)) {
                                word |= static_cast<Word>(Word{1} << i);
                        }
                }
                return word;
        }

        template<class charT>
        static constexpr auto invalid_argument(
                charT ch, charT zero = static_cast<charT>('0'), charT one = static_cast<charT>('1'),
                std::source_location const& loc = std::source_location::current()
        )
        {
                return std::invalid_argument(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: invalid argument ‘ch‘ [{} != {} or {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), ch, zero, one
                        )
                );
        }

        [[nodiscard]] constexpr auto out_of_range(std::size_t pos, std::source_location const& loc = std::source_location::current()) const
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘pos‘ is out of range [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), pos, size()
                        )
                );
        }

        [[nodiscard]] static constexpr auto overflow_error(std::source_location const& loc = std::source_location::current())
        {
                return std::overflow_error(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: a set position lies beyond the word",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name()
                        )
                );
        }
};

// The owner's side of the view protocol: what a bit_set_view or bit_span over a bitset refers into. [design.md#views-over-owners]
template<class Bits, class Traits>
struct owned_storage<bitset_adaptor<Bits, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;
};

}       // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// bitset hash support [bitset.hash]; no redeclaration of std::hash's primary template, which [namespace.std] forbids.
template<class Bits, class Traits>
struct hash<xstd::bitset_adaptor<Bits, Traits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bitset_adaptor<Bits, Traits> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

// NOLINTEND(bugprone-std-namespace-modification)

}       // namespace std

namespace xstd {

// bitset operators                                           [bitset.operators]
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator&(bitset_adaptor<Bits, Traits> const& lhs, bitset_adaptor<Bits, Traits> const& rhs) noexcept(static_bit_extent<Traits, Bits>) -> bitset_adaptor<Bits, Traits> { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator|(bitset_adaptor<Bits, Traits> const& lhs, bitset_adaptor<Bits, Traits> const& rhs) noexcept(static_bit_extent<Traits, Bits>) -> bitset_adaptor<Bits, Traits> { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator^(bitset_adaptor<Bits, Traits> const& lhs, bitset_adaptor<Bits, Traits> const& rhs) noexcept(static_bit_extent<Traits, Bits>) -> bitset_adaptor<Bits, Traits> { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator-(bitset_adaptor<Bits, Traits> const& lhs, bitset_adaptor<Bits, Traits> const& rhs) noexcept(static_bit_extent<Traits, Bits>) -> bitset_adaptor<Bits, Traits> requires requires (bitset_adaptor<Bits, Traits>& b) { b -= b; } { auto nrv = lhs; nrv -= rhs; return nrv; }

// [bitset.operators]/6: up to N characters into a temporary string, then x = bitset(str), so a short read lands in the low bits as it does there;
// a run-time width reads every 0 or 1 on offer and is as wide as the characters read, as boost's is.
template<class charT, class traits, class Bits, class Traits>
auto operator>>(std::basic_istream<charT, traits>& is, bitset_adaptor<Bits, Traits>& x)
        -> std::basic_istream<charT, traits>&
{
        auto const limit = [&] -> std::size_t {
                if constexpr (static_bit_extent<Traits, Bits>) {
                        return x.size();
                } else {
                        return std::numeric_limits<std::size_t>::max();
                }
        }();
        auto str = std::basic_string<charT, traits>();
        // Assigned inside an if constexpr the zero-width instantiation discards. [design.md#clang-tidy-false-positives]
        auto state = std::ios_base::goodbit;  // NOLINT(misc-const-correctness)
        charT ch;
        // One peek per character: peeking twice sets eofbit then failbit, failing a short but valid extraction ([bitset.operators]/6).
        while (str.size() < limit) {
                auto const next = is.peek();
                if (not traits::eq_int_type(next, is.widen('0')) and not traits::eq_int_type(next, is.widen('1'))) {
                        break;
                }
                is >> ch;
                str.push_back(ch);
        }
        x = bitset_adaptor<Bits, Traits>(str);
        if constexpr (not detail::bits::zero_width<Traits>) {
                if (str.empty()) {
                        state |= std::ios_base::failbit;
                        is.setstate(state);
                }
        }
        return is;
}

template<class charT, class traits, class Bits, class Traits>
auto operator<<(std::basic_ostream<charT, traits>& os, bitset_adaptor<Bits, Traits> const& x)
        -> std::basic_ostream<charT, traits>&
{
        return os << x.template to_string<charT, traits, std::allocator<charT>>(
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('0'),
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('1')
        );
}

}       // namespace xstd

#endif // XSTD_BITS_BITSET_ADAPTOR_HPP
