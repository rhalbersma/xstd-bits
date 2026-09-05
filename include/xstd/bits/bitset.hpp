//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_HPP
#define XSTD_BITS_BITSET_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <iosfwd>                                  // basic_istream, basic_ostream
#include <string>                                  // basic_string, char_traits

#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cstddef>                                 // size_t

namespace xstd {

template<std::size_t N, xstd::unsigned_integer Block = std::size_t> class bitset;

template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator&(const bitset<N, Block>&, const bitset<N, Block>&) noexcept -> bitset<N, Block>;
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator|(const bitset<N, Block>&, const bitset<N, Block>&) noexcept -> bitset<N, Block>;
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator^(const bitset<N, Block>&, const bitset<N, Block>&) noexcept -> bitset<N, Block>;
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator-(const bitset<N, Block>&, const bitset<N, Block>&) noexcept -> bitset<N, Block>;

template<class charT, class traits, std::size_t N, xstd::unsigned_integer Block> auto operator>>(std::basic_istream<charT, traits>& is,       bitset<N, Block>& x) -> std::basic_istream<charT, traits>&;
template<class charT, class traits, std::size_t N, xstd::unsigned_integer Block> auto operator<<(std::basic_ostream<charT, traits>& os, const bitset<N, Block>& x) -> std::basic_ostream<charT, traits>&;

}       // namespace xstd

#include <boost/hash2/fnv1a.hpp>                   // fnv1a_64
#include <boost/hash2/hash_append.hpp>             // hash_append
#include <xstd/bits/bit_traits.hpp>                // bit_traits
#include <xstd/bits/block_sequence.hpp>            // block_array
#include <xstd/bits/ranges/bit_extent.hpp>         // bit_extent
#include <xstd/bits/ranges/sequence_view.hpp>      // sequence_find
#include <xstd/bits/ranges/set_view.hpp>           // set_find, set_compare
#include <algorithm>                               // find_if, min
#include <cassert>                                 // assert
#include <compare>                                 // strong_ordering
#include <concepts>                                // unsigned_integral
#include <format>                                  // format
#include <functional>                              // hash
#include <ios>                                     // ios_base
#include <locale>                                  // ctype, use_facet
#include <memory>                                  // allocator
#include <ranges>                                  // find_if, iota, reverse
#include <source_location>                         // source_location
#include <stdexcept>                               // invalid_argument, out_of_range
#include <string_view>                             // basic_string_view
#include <utility>                                 // as_const

// Class template bitset [template.bitset], General [template.bitset.general]

namespace xstd {

template<std::size_t N, xstd::unsigned_integer Block>
class bitset
{
        // No iteration and no <=> here by design, because std::bitset has neither: choose set_view or sequence_view.
        block_array<Block, N> m_bits{};

        // The door reaches the storage; nothing else needs to. [design.md#the-door]
        friend struct bit_traits<bitset>;

        // ADL rather than a specialization, because this type is ours to add hidden friends to.
        [[nodiscard]] friend constexpr auto block_count(const bitset& c) noexcept -> std::size_t { return c.m_bits.num_blocks(); }
        [[nodiscard]] friend constexpr auto block_at(const bitset& c, std::size_t i) noexcept -> Block { return c.m_bits.block(i); }

        template<class Provider, class Hash, class Flavor>
        friend constexpr void tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bitset const* v) noexcept
        {
                boost::hash2::hash_append(h, f, v->m_bits);
        }

public:
        using block_type = Block;

        // sequence_view's proxy as [bitset.refs] asks for it, reaching the bits only through the unchecked way in. [design.md#unchecked-writes-in-views]
        class reference
        {
                // A pointer, not a reference, so the copy constructor stays defaulted as [bitset.refs] declares it.
                bitset* m_ptr{};
                std::size_t m_idx{};

                friend bitset;

                [[nodiscard]] constexpr reference(bitset& c, std::size_t idx) noexcept
                :
                        m_ptr(&c),
                        m_idx(idx)
                {}

        public:
                constexpr reference(const reference& x) noexcept = default;
                constexpr ~reference() = default;

                constexpr auto operator=(bool x) noexcept -> reference&
                {
                        std::as_const(*this) = x;
                        return *this;
                }

                // Assigns the bit, not the proxy: rebinding would break the swap below. [design.md#clang-tidy-false-positives]
                constexpr auto operator=(const reference& x) noexcept -> reference&  // NOLINT(bugprone-unhandled-self-assignment)
                {
                        return *this = static_cast<bool>(x);
                }

                // A proxy reference assigns through a const proxy, the shape the standard gives vector<bool>::reference.
                constexpr auto operator=(bool x) const noexcept -> const reference&  // NOLINT(misc-unconventional-assign-operator)
                {
                        if (x) {
                                m_ptr->m_bits.set(m_idx);
                        } else {
                                m_ptr->m_bits.reset(m_idx);
                        }
                        return *this;
                }

                [[nodiscard]] constexpr explicit(false) operator bool() const noexcept  // NOLINT(misc-explicit-constructor)
                {
                        return m_ptr->m_bits.test(m_idx);
                }

                [[nodiscard]] constexpr auto operator~() const noexcept -> bool
                {
                        return not m_ptr->m_bits.test(m_idx);
                }

                friend constexpr void swap(reference x, reference y) noexcept { bool const t = x; x = y; y = t; }
                friend constexpr void swap(reference x,     bool& y) noexcept { bool const t = x; x = y; y = t; }
                friend constexpr void swap(    bool& x, reference y) noexcept { bool const t = x; x = y; y = t; }

                constexpr auto flip() noexcept -> reference&
                {
                        m_ptr->m_bits.flip(m_idx);
                        return *this;
                }
        };

        // Constructors                                            [bitset.cons]
        [[nodiscard]] constexpr bitset() noexcept = default;

        [[nodiscard]] constexpr explicit(false) bitset(unsigned long long val) noexcept = delete;       // TODO

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit bitset(
                const std::basic_string<charT, traits, Allocator>& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = charT('0'),
                charT one  = charT('1')
        )
        :
                bitset(std::basic_string_view<charT, traits>(str), pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit bitset(
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
                auto const M = std::ranges::min(N, rlen);
                for (auto const i : std::views::iota(0UZ, M)) {
                        auto const ch = str[pos + M - 1 - i];
                        if (traits::eq(ch, zero)) {
                                continue;
                        }
                        if (traits::eq(ch, one)) {
                                m_bits.set(i);
                        } else {
                                throw invalid_argument(ch, zero, one);
                        }
                }
        }

        template<class charT>
        [[nodiscard]] constexpr explicit bitset(
                const charT* str,
                std::basic_string_view<charT>::size_type n = std::basic_string_view<charT>::npos,
                charT zero = charT('0'),
                charT one  = charT('1')
        )
        :
                bitset(n == std::basic_string_view<charT>::npos ? std::basic_string_view<charT>(str) : std::basic_string_view<charT>(str, n), 0, n, zero, one)
        {}

        // Members                                              [bitset.members]
        constexpr auto operator&=(const bitset& rhs) noexcept -> bitset& { m_bits &= rhs.m_bits; return *this; }
        constexpr auto operator|=(const bitset& rhs) noexcept -> bitset& { m_bits |= rhs.m_bits; return *this; }
        constexpr auto operator^=(const bitset& rhs) noexcept -> bitset& { m_bits ^= rhs.m_bits; return *this; }
        constexpr auto operator-=(const bitset& rhs) noexcept -> bitset& { m_bits -= rhs.m_bits; return *this; }

        constexpr auto operator<<=(std::size_t pos) noexcept
                -> bitset&
        {
                if (pos < N) {
                        m_bits <<= pos;
                } else {
                        m_bits.reset();
                }
                return *this;
        }

        constexpr auto operator>>=(std::size_t pos) noexcept
                -> bitset&
        {
                if (pos < N) {
                        m_bits >>= pos;
                } else {
                        m_bits.reset();
                }
                return *this;
        }

        [[nodiscard]] constexpr auto operator<<(std::size_t pos) const noexcept -> bitset { auto nrv = *this; nrv <<= pos; return nrv; }
        [[nodiscard]] constexpr auto operator>>(std::size_t pos) const noexcept -> bitset { auto nrv = *this; nrv >>= pos; return nrv; }

        [[nodiscard]] constexpr auto operator~() const noexcept -> bitset { auto nrv = *this; nrv.flip(); return nrv; }

        constexpr auto set  () noexcept -> bitset& { m_bits.set  (); return *this; }
        constexpr auto reset() noexcept -> bitset& { m_bits.reset(); return *this; }
        constexpr auto flip () noexcept -> bitset& { m_bits.flip (); return *this; }

        constexpr auto set(std::size_t pos, bool val = true)
                -> bitset&
        {
                if (pos < N) {
                        if (val) {
                                m_bits.set(pos);
                        } else {
                                m_bits.reset(pos);
                        }
                        return *this;
                }
                throw out_of_range(pos);
        }

        constexpr auto reset(std::size_t pos)
                -> bitset&
        {
                if (pos < N) {
                        m_bits.reset(pos);
                        return *this;
                }
                throw out_of_range(pos);
        }

        constexpr auto flip(std::size_t pos)
                -> bitset&
        {
                if (pos < N) {
                        m_bits.flip(pos);
                        return *this;
                }
                throw out_of_range(pos);
        }

        [[nodiscard]] constexpr auto operator[](std::size_t pos) const noexcept
                -> bool
        {
                return m_bits.test(pos);
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
                // The finding is the N == 0 instantiation, where empty is the answer. [design.md#clang-tidy-false-positives]
                auto str = std::basic_string<charT, traits, Allocator>(N, zero);  // NOLINT(bugprone-string-constructor)
                for (auto const i : std::views::iota(0UZ, N)) {
                        if (m_bits.test(N - 1 - i)) {
                                str[i] = one;
                        }
                }
                return str;
        }

        // observers
        [[nodiscard]] constexpr auto count() const noexcept -> std::size_t { return m_bits.count(); }
        [[nodiscard]] constexpr auto size()  const noexcept -> std::size_t { return m_bits.size();  }

        [[nodiscard]] constexpr auto operator==(const bitset& rhs) const noexcept -> bool = default;

        [[nodiscard]] constexpr auto test(std::size_t pos) const
                -> bool
        {
                if (pos < N) {
                        return m_bits.test(pos);
                }
                throw out_of_range(pos);
        }

        [[nodiscard]] constexpr auto all()  const noexcept -> bool { return m_bits.all();  }
        [[nodiscard]] constexpr auto any()  const noexcept -> bool { return m_bits.any();  }
        [[nodiscard]] constexpr auto none() const noexcept -> bool { return m_bits.none(); }

        [[nodiscard]] constexpr auto is_subset_of       (const bitset& rhs) const noexcept -> bool { return m_bits.is_subset_of       (rhs.m_bits); }
        [[nodiscard]] constexpr auto is_proper_subset_of(const bitset& rhs) const noexcept -> bool { return m_bits.is_proper_subset_of(rhs.m_bits); }
        [[nodiscard]] constexpr auto intersects         (const bitset& rhs) const noexcept -> bool { return m_bits.intersects         (rhs.m_bits); }

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

        static constexpr auto out_of_range(std::size_t pos, std::source_location const& loc = std::source_location::current())
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘pos‘ is out of range [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), pos, N
                        )
                );
        }
};


// Each entry unwraps to the storage's own door; one friend declaration replaces the two hidden friends. [design.md#the-door]
template<std::size_t N, xstd::unsigned_integer Block>
struct bit_traits<bitset<N, Block>>
{
        using bits_type = bitset<N, Block>;
        using backend   = bit_traits<block_array<Block, N>>;

        static constexpr std::size_t extent = N;

        [[nodiscard]] static constexpr auto size (bits_type const& c)                noexcept -> std::size_t { return backend::size(c.m_bits);  }
        [[nodiscard]] static constexpr auto at   (bits_type const& c, std::size_t n) noexcept -> bool        { return backend::at(c.m_bits, n); }
        [[nodiscard]] static constexpr auto count(bits_type const& c)                noexcept -> std::size_t { return backend::count(c.m_bits); }

        static constexpr void assign(bits_type& c, std::size_t n, bool value) noexcept { backend::assign(c.m_bits, n, value); }
        static constexpr void insert(bits_type& c, std::size_t n)             noexcept { backend::insert(c.m_bits, n);        }
        static constexpr void fill  (bits_type& c, bool value)                noexcept { backend::fill(c.m_bits, value);      }

        [[nodiscard]] static constexpr auto num_blocks(bits_type const& c)                noexcept -> std::size_t { return backend::num_blocks(c.m_bits); }
        [[nodiscard]] static constexpr auto block     (bits_type const& c, std::size_t i) noexcept                { return backend::block(c.m_bits, i);   }

        [[nodiscard]] static constexpr auto find_first(bits_type const& c)                noexcept -> std::size_t { return backend::find_first(c.m_bits);   }
        [[nodiscard]] static constexpr auto find_last (bits_type const& c)                noexcept -> std::size_t { return backend::find_last(c.m_bits);    }
        [[nodiscard]] static constexpr auto find_next (bits_type const& c, std::size_t n) noexcept -> std::size_t { return backend::find_next(c.m_bits, n); }
        [[nodiscard]] static constexpr auto find_prev (bits_type const& c, std::size_t n) noexcept -> std::size_t { return backend::find_prev(c.m_bits, n); }

        [[nodiscard]] static constexpr auto set_three_way     (bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return backend::set_three_way(x.m_bits, y.m_bits);      }
        [[nodiscard]] static constexpr auto sequence_three_way(bits_type const& x, bits_type const& y) noexcept -> std::strong_ordering { return backend::sequence_three_way(x.m_bits, y.m_bits); }
};

}       // namespace xstd

// Specializing set_find is what lets set_view(x) work, and it scans public operator[], claiming no privilege std::bitset lacks.
namespace xstd::ranges {

// As for std::bitset<N>: the width is in the type.
template<std::size_t N, xstd::unsigned_integer Block>
inline constexpr std::size_t bit_extent<xstd::bitset<N, Block>> = N;

template<std::size_t N, xstd::unsigned_integer Block>
struct set_find<xstd::bitset<N, Block>>
{
        [[nodiscard]] static constexpr auto first(xstd::bitset<N, Block> const& c) noexcept
                -> std::size_t
        {
                if constexpr (N == 0) {
                        return N;
                } else {
                        return *std::ranges::find_if(std::views::iota(0UZ, N), [&](auto i) {
                                return c[i];
                        });
                }
        }

        [[nodiscard]] static constexpr auto last(xstd::bitset<N, Block> const&) noexcept
                -> std::size_t
        {
                return N;
        }

        [[nodiscard]] static constexpr auto next(xstd::bitset<N, Block> const& c, std::size_t n) noexcept
                -> std::size_t
        {
                return *std::ranges::find_if(std::views::iota(n + 1, N), [&](auto i) {
                        return c[i];
                });
        }

        [[nodiscard]] static auto prev(xstd::bitset<N, Block> const& c, std::size_t n) noexcept
                -> std::size_t
        {
                assert(c.any());
                return *std::ranges::find_if(std::views::iota(0UZ, n) | std::views::reverse, [&](auto i) {
                        return c[i];
                });
        }
};

// The other reading, trivial because operator[] answers every position; without it we would be a set and not a sequence.
template<std::size_t N, xstd::unsigned_integer Block>
struct sequence_find<xstd::bitset<N, Block>>
{
        [[nodiscard]] static constexpr auto first(xstd::bitset<N, Block> const&) noexcept -> std::size_t { return 0UZ; }
        [[nodiscard]] static constexpr auto last (xstd::bitset<N, Block> const&) noexcept -> std::size_t { return N;   }
        [[nodiscard]] static constexpr auto at   (xstd::bitset<N, Block> const& c, std::size_t n) noexcept -> bool { return c[n]; }
};

// No <=> of its own, so opt in to the iteration-based ordering explicitly, as std::bitset and dynamic_bitset do.
template<std::size_t N, xstd::unsigned_integer Block>
struct set_compare<xstd::bitset<N, Block>>
{
        [[nodiscard]] static constexpr auto lexicographical_three_way(xstd::bitset<N, Block> const& x, xstd::bitset<N, Block> const& y) noexcept
                -> std::strong_ordering
        {
                return set_three_way(set_view(x), set_view(y));
        }
};

}       // namespace xstd::ranges

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// bitset hash support [bitset.hash]; no redeclaration of std::hash's primary template, which [namespace.std] forbids.
template<size_t N, unsigned_integral Block>
struct hash<xstd::bitset<N, Block>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bitset<N, Block> const& v) const noexcept
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
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator&(const bitset<N, Block>& lhs, const bitset<N, Block>& rhs) noexcept -> bitset<N, Block> { auto nrv = lhs; nrv &= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator|(const bitset<N, Block>& lhs, const bitset<N, Block>& rhs) noexcept -> bitset<N, Block> { auto nrv = lhs; nrv |= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator^(const bitset<N, Block>& lhs, const bitset<N, Block>& rhs) noexcept -> bitset<N, Block> { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<std::size_t N, xstd::unsigned_integer Block> [[nodiscard]] constexpr auto operator-(const bitset<N, Block>& lhs, const bitset<N, Block>& rhs) noexcept -> bitset<N, Block> { auto nrv = lhs; nrv -= rhs; return nrv; }

template<class charT, class traits, std::size_t N, xstd::unsigned_integer Block>
auto operator>>(std::basic_istream<charT, traits>& is, bitset<N, Block>& x)
        -> std::basic_istream<charT, traits>&
{
        auto str = std::basic_string<charT, traits>(N, is.widen('0'));  // NOLINT(bugprone-string-constructor)
        // Assigned inside an if constexpr the N == 0 instantiation discards. [design.md#clang-tidy-false-positives]
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
        x = bitset<N, Block>(str);
        if constexpr (N > 0) {
                if (i == 0) {
                        state |= std::ios_base::failbit;
                        is.setstate(state);
                }
        }
        return is;
}

template<class charT, class traits, std::size_t N, xstd::unsigned_integer Block>
auto operator<<(std::basic_ostream<charT, traits>& os, const bitset<N, Block>& x)
        -> std::basic_ostream<charT, traits>&
{
        return os << x.template to_string<charT, traits, std::allocator<charT>>(
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('0'),
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('1')
        );
}

}       // namespace xstd

#endif // XSTD_BITS_BITSET_HPP
