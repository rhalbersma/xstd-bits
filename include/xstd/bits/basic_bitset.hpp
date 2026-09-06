//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BASIC_BITSET_HPP
#define XSTD_BITS_BASIC_BITSET_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <boost/hash2/fnv1a.hpp>              // fnv1a_64
#include <boost/hash2/hash_append.hpp>        // hash_append
#include <xstd/bits/bit_traits.hpp>           // bit_storage, bit_traits, block_readable, find_first, find_next, find_prev, static_bit_extent, zero_width
#include <xstd/bits/ranges/bit_extent.hpp>    // bit_extent
#include <xstd/bits/ranges/sequence_view.hpp> // sequence_find
#include <xstd/bits/ranges/set_view.hpp>      // set_compare, set_three_way, set_view
#include <algorithm>                          // min
#include <compare>                            // strong_ordering
#include <concepts>                           // convertible_to, regular, same_as
#include <cstddef>                            // size_t
#include <format>                             // format
#include <functional>                         // hash
#include <ios>                                // ios_base
#include <iosfwd>                             // basic_istream, basic_ostream
#include <locale>                             // ctype, use_facet
#include <memory>                             // allocator
#include <ranges>                             // iota
#include <source_location>                    // source_location
#include <stdexcept>                          // invalid_argument, out_of_range
#include <string>                             // basic_string, char_traits
#include <string_view>                        // basic_string_view
#include <utility>                            // as_const

namespace xstd {

// The bitset vocabulary a storage speaks natively, one line each in the wrapper; a backend missing a member fails here, at the class.
// The shifts stay in although the door carries their contracts: without them a shiftless backend would fail inside an instantiation. [design.md#the-idempotent-wrapper]
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

// [template.bitset] over any Bits that speaks the vocabulary: what Bits has is forwarded, what it lacks is added through the door. [design.md#the-idempotent-wrapper]
template<has_bitops Bits, bit_storage<Bits> Traits = bit_traits<Bits>>
class basic_bitset
{
        static_assert(static_bit_extent<Traits, Bits>, "a dynamic width arrives with block_vector, in step 7 of #80");

        // No iteration and no <=> here by design, because std::bitset has neither: choose set_view or sequence_view.
        Bits m_bits{};

        // Still published for set_view and block_range, until the rewire routes them through the door. [design.md#the-iterator-is-the-primitive]
        [[nodiscard]] friend constexpr auto block_count(basic_bitset const& c) noexcept -> std::size_t requires block_readable<Traits, Bits> { return Traits::num_blocks(c.m_bits); }
        [[nodiscard]] friend constexpr auto block_at(basic_bitset const& c, std::size_t i) noexcept requires block_readable<Traits, Bits> { return Traits::block(c.m_bits, i); }

        [[nodiscard]] friend constexpr auto find_first(basic_bitset const& c)                noexcept -> std::size_t { return detail::bits::find_first<Traits>(c.m_bits);   }
        [[nodiscard]] friend constexpr auto find_last (basic_bitset const& c)                noexcept -> std::size_t { return Traits::size(c.m_bits);                       }
        [[nodiscard]] friend constexpr auto find_next (basic_bitset const& c, std::size_t n) noexcept -> std::size_t { return detail::bits::find_next<Traits>(c.m_bits, n); }
        [[nodiscard]] friend constexpr auto find_prev (basic_bitset const& c, std::size_t n) noexcept -> std::size_t { return detail::bits::find_prev<Traits>(c.m_bits, n); }

        // The set ordering for set_view's <=>, the door's word-wise entry where it has one, the iteration otherwise. [design.md#two-readings-disagree]
        [[nodiscard]] friend constexpr auto set_three_way(basic_bitset const& x, basic_bitset const& y) noexcept
                -> std::strong_ordering
        {
                if constexpr (requires { Traits::set_three_way(x.m_bits, y.m_bits); }) {
                        return Traits::set_three_way(x.m_bits, y.m_bits);
                } else {
                        return xstd::ranges::set_three_way(xstd::ranges::set_view(x), xstd::ranges::set_view(y));
                }
        }

        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, basic_bitset const* v) noexcept
        {
                boost::hash2::hash_append(h, f, v->m_bits);
        }

public:
        // [bitset.refs], reaching the bits only through the unchecked way in; its own class, with the flip and ~ the sequence proxy lacks. [design.md#unchecked-writes-in-views]
        class reference
        {
                // A pointer, not a reference, so the copy constructor stays defaulted as [bitset.refs] declares it.
                basic_bitset* m_ptr{};
                std::size_t m_idx{};

                friend basic_bitset;

                [[nodiscard]] constexpr reference(basic_bitset& c, std::size_t idx) noexcept
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
                        Traits::assign(m_ptr->m_bits, m_idx, x);
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
                        Traits::assign(m_ptr->m_bits, m_idx, not Traits::at(m_ptr->m_bits, m_idx));
                        return *this;
                }
        };

        // Constructors                                            [bitset.cons]
        [[nodiscard]] constexpr basic_bitset() noexcept = default;
        [[nodiscard]] constexpr explicit(false) basic_bitset(unsigned long long val) noexcept = delete;       // TODO

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit basic_bitset(
                std::basic_string<charT, traits, Allocator> const& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = charT('0'),
                charT one  = charT('1')
        )
        :
                basic_bitset(std::basic_string_view<charT, traits>(str), pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit basic_bitset(
                std::basic_string_view<charT, traits> str,
                std::basic_string_view<charT, traits>::size_type pos = 0,
                std::basic_string_view<charT, traits>::size_type n = std::basic_string_view<charT, traits>::npos,
                charT zero = charT('0'),
                charT one  = charT('1')
        )
        {
                if (pos > str.size()) {
                        throw out_of_range(pos);
                }
                auto const rlen = std::ranges::min(n, str.size() - pos);
                auto const M = std::ranges::min(size(), rlen);
                for (auto const i : std::views::iota(0UZ, M)) {
                        auto const ch = str[pos + M - 1 - i];
                        if (traits::eq(ch, zero)) {
                                continue;
                        }
                        if (traits::eq(ch, one)) {
                                Traits::assign(m_bits, i, true);
                        } else {
                                throw invalid_argument(ch, zero, one);
                        }
                }
        }

        template<class charT>
        [[nodiscard]] constexpr explicit basic_bitset(
                charT const* str,
                std::basic_string_view<charT>::size_type n = std::basic_string_view<charT>::npos,
                charT zero = charT('0'),
                charT one  = charT('1')
        )
        :
                basic_bitset(n == std::basic_string_view<charT>::npos ? std::basic_string_view<charT>(str) : std::basic_string_view<charT>(str, n), 0, n, zero, one)
        {}

        // Members                                              [bitset.members]
        constexpr auto operator&=(basic_bitset const& rhs) noexcept -> basic_bitset& { m_bits &= rhs.m_bits; return *this; }
        constexpr auto operator|=(basic_bitset const& rhs) noexcept -> basic_bitset& { m_bits |= rhs.m_bits; return *this; }
        constexpr auto operator^=(basic_bitset const& rhs) noexcept -> basic_bitset& { m_bits ^= rhs.m_bits; return *this; }

        // Same spelling, two contracts: the door's checked entry is total and forwarded as is; the storage's own shift is the unchecked one, guarded here. [design.md#checked-and-unchecked]
        constexpr auto operator<<=(std::size_t pos) noexcept
                -> basic_bitset&
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
                -> basic_bitset&
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

        [[nodiscard]] constexpr auto operator<<(std::size_t pos) const noexcept -> basic_bitset { auto nrv = *this; nrv <<= pos; return nrv; }
        [[nodiscard]] constexpr auto operator>>(std::size_t pos) const noexcept -> basic_bitset { auto nrv = *this; nrv >>= pos; return nrv; }

        [[nodiscard]] constexpr auto operator~() const noexcept -> basic_bitset { auto nrv = *this; nrv.flip(); return nrv; }

        constexpr auto set  () noexcept -> basic_bitset& { m_bits.set  (); return *this; }
        constexpr auto reset() noexcept -> basic_bitset& { m_bits.reset(); return *this; }
        constexpr auto flip () noexcept -> basic_bitset& { m_bits.flip (); return *this; }

        // Element access, both families: the door's checked entry where the counterpart throws natively, else the guard and the unchecked write. [design.md#checked-and-unchecked]
        constexpr auto set(std::size_t pos, bool val = true)
                -> basic_bitset&
        {
                if constexpr (requires { Traits::checked_set(m_bits, pos, val); }) {
                        Traits::checked_set(m_bits, pos, val);
                } else if (pos < size()) {
                        Traits::assign(m_bits, pos, val);
                } else {
                        throw out_of_range(pos);
                }
                return *this;
        }

        constexpr auto reset(std::size_t pos)
                -> basic_bitset&
        {
                if constexpr (requires { Traits::checked_reset(m_bits, pos); }) {
                        Traits::checked_reset(m_bits, pos);
                } else if (pos < size()) {
                        Traits::assign(m_bits, pos, false);
                } else {
                        throw out_of_range(pos);
                }
                return *this;
        }

        constexpr auto flip(std::size_t pos)
                -> basic_bitset&
        {
                if constexpr (requires { Traits::checked_flip(m_bits, pos); }) {
                        Traits::checked_flip(m_bits, pos);
                } else if (pos < size()) {
                        Traits::assign(m_bits, pos, not Traits::at(m_bits, pos));
                } else {
                        throw out_of_range(pos);
                }
                return *this;
        }

        // The const subscript is unchecked on every counterpart, so it is the door's at() unconditionally.
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

        [[nodiscard]] constexpr auto to_ulong()  const -> unsigned long      = delete;  // TODO
        [[nodiscard]] constexpr auto to_ullong() const -> unsigned long long = delete;  // TODO

        template<
                class charT = char,
                class traits = std::char_traits<charT>,
                class Allocator = std::allocator<charT>
        >
        [[nodiscard]] constexpr auto to_string(charT zero = charT('0'), charT one = charT('1')) const
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

        [[nodiscard]] constexpr auto operator==(basic_bitset const& rhs) const noexcept -> bool = default;

        [[nodiscard]] constexpr auto test(std::size_t pos) const
                -> bool
        {
                if constexpr (requires { { Traits::checked_test(m_bits, pos) } -> std::convertible_to<bool>; }) {
                        return Traits::checked_test(m_bits, pos);
                } else if (pos < size()) {
                        return Traits::at(m_bits, pos);
                } else {
                        throw out_of_range(pos);
                }
        }

        [[nodiscard]] constexpr auto all()  const noexcept -> bool { return m_bits.all();  }
        [[nodiscard]] constexpr auto any()  const noexcept -> bool { return m_bits.any();  }
        [[nodiscard]] constexpr auto none() const noexcept -> bool { return m_bits.none(); }

        // No -=, is_subset_of, is_proper_subset_of or intersects at a static width: std::bitset has none, and set_view keeps them. [design.md#the-idempotent-wrapper]

private:
        template<class charT>
        static constexpr auto invalid_argument(
                charT ch, charT zero = charT('0'), charT one = charT('1'),
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
};

}       // namespace xstd

// The two readings and the width, published to the views the way the hidden friends above are: until the rewire.
namespace xstd::ranges {

template<class Bits, class Traits>
inline constexpr std::size_t bit_extent<xstd::basic_bitset<Bits, Traits>> = Traits::extent;

// The sequence reading, trivial because the const subscript answers every position; without it we would be a set and not a sequence.
template<class Bits, class Traits>
struct sequence_find<xstd::basic_bitset<Bits, Traits>>
{
        [[nodiscard]] static constexpr auto first(xstd::basic_bitset<Bits, Traits> const&)                  noexcept -> std::size_t { return 0UZ;      }
        [[nodiscard]] static constexpr auto last (xstd::basic_bitset<Bits, Traits> const& c)                noexcept -> std::size_t { return c.size(); }
        [[nodiscard]] static constexpr auto at   (xstd::basic_bitset<Bits, Traits> const& c, std::size_t n) noexcept -> bool        { return c[n];     }
};

// No <=> of its own, so opt in to the set ordering explicitly, as std::bitset and dynamic_bitset do.
template<class Bits, class Traits>
struct set_compare<xstd::basic_bitset<Bits, Traits>>
{
        [[nodiscard]] static constexpr auto lexicographical_three_way(xstd::basic_bitset<Bits, Traits> const& x, xstd::basic_bitset<Bits, Traits> const& y) noexcept
                -> std::strong_ordering
        {
                return set_three_way(x, y);
        }
};

}       // namespace xstd::ranges

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// bitset hash support [bitset.hash]; no redeclaration of std::hash's primary template, which [namespace.std] forbids.
template<class Bits, class Traits>
struct hash<xstd::basic_bitset<Bits, Traits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::basic_bitset<Bits, Traits> const& v) const noexcept
                -> std::size_t
        {
                boost::hash2::fnv1a_64 h;
                boost::hash2::hash_append(h, {}, v);
                return boost::hash2::get_integral_result<std::size_t>(h);
        }
};

// NOLINTEND(bugprone-std-namespace-modification)

}       // namespace std

namespace xstd {

// bitset operators                                           [bitset.operators]
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator&(basic_bitset<Bits, Traits> const& lhs, basic_bitset<Bits, Traits> const& rhs) noexcept -> basic_bitset<Bits, Traits> { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator|(basic_bitset<Bits, Traits> const& lhs, basic_bitset<Bits, Traits> const& rhs) noexcept -> basic_bitset<Bits, Traits> { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator^(basic_bitset<Bits, Traits> const& lhs, basic_bitset<Bits, Traits> const& rhs) noexcept -> basic_bitset<Bits, Traits> { auto nrv = lhs; nrv ^= rhs; return nrv; }

template<class charT, class traits, class Bits, class Traits>
auto operator>>(std::basic_istream<charT, traits>& is, basic_bitset<Bits, Traits>& x)
        -> std::basic_istream<charT, traits>&
{
        auto const N = x.size();
        auto str = std::basic_string<charT, traits>(N, is.widen('0'));  // NOLINT(bugprone-string-constructor)
        // Assigned inside an if constexpr the zero-width instantiation discards. [design.md#clang-tidy-false-positives]
        auto state = std::ios_base::goodbit;  // NOLINT(misc-const-correctness)
        charT ch;
        auto i = 0UZ;
        // One peek per character: peeking twice sets eofbit then failbit, failing a short but valid extraction ([bitset.operators]/6).
        while (i < N) {
                auto const next = is.peek();
                if (not traits::eq_int_type(next, is.widen('0')) and not traits::eq_int_type(next, is.widen('1'))) {
                        break;
                }
                is >> ch;
                if (traits::eq(ch, is.widen('1'))) {
                        str[i] = ch;
                }
                ++i;
        }
        x = basic_bitset<Bits, Traits>(str);
        if constexpr (not detail::bits::zero_width<Traits>) {
                if (i == 0) {
                        state |= std::ios_base::failbit;
                        is.setstate(state);
                }
        }
        return is;
}

template<class charT, class traits, class Bits, class Traits>
auto operator<<(std::basic_ostream<charT, traits>& os, basic_bitset<Bits, Traits> const& x)
        -> std::basic_ostream<charT, traits>&
{
        return os << x.template to_string<charT, traits, std::allocator<charT>>(
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('0'),
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('1')
        );
}

}       // namespace xstd

#endif // XSTD_BITS_BASIC_BITSET_HPP
