//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_ADAPTOR_HPP
#define XSTD_BITS_BITSET_ADAPTOR_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <xstd/bits/bit_traits.hpp>               // bit_storage, bit_traits, block_readable, scan_prev, static_bit_extent, word_at, zero_width
#include <xstd/bits/detail/allocator_typedef.hpp> // allocator_typedef
#include <xstd/bits/detail/hash.hpp>              // hash_append_bits, std_hash
#include <xstd/bits/ownership.hpp>                // owned_storage, ownership
#include <boost/hash2/hash_append.hpp>            // hash_append_tag
#include <algorithm>                              // min
#include <cassert>                                // assert
#include <compare>                                // strong_ordering
#include <concepts>                               // convertible_to, regular, same_as, swappable
#include <cstddef>                                // size_t
#include <format>                                 // format
#include <functional>                             // hash
#include <ios>                                    // ios_base
#include <iosfwd>                                 // basic_istream, basic_ostream
#include <iterator>                               // input_iterator, iter_value_t, output_iterator, sentinel_for
#include <limits>                                 // numeric_limits
#include <locale>                                 // ctype, use_facet
#include <memory>                                 // allocator
#include <ranges>                                 // iota, swap
#include <source_location>                        // source_location
#include <stdexcept>                              // invalid_argument, out_of_range, overflow_error
#include <string>                                 // basic_string, char_traits
#include <string_view>                            // basic_string_view
#include <type_traits>                            // is_nothrow_swappable_v, remove_cvref_t
#include <utility>                                // as_const

namespace xstd {

// The bitset vocabulary the storage speaks natively, one line each in the wrapper: std::bitset's members and boost's set vocabulary, so a storage missing one fails here, at the class. [design.md#a-strict-extension]
template<class Bits>
concept has_bitops =
        std::regular<Bits> and
        requires (Bits& b, Bits const& c, std::size_t n)
        {
                { b &= c    } -> std::same_as<Bits&>;
                { b |= c    } -> std::same_as<Bits&>;
                { b ^= c    } -> std::same_as<Bits&>;
                { b -= c    } -> std::same_as<Bits&>;
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
                { c.is_subset_of(c)        } -> std::same_as<bool>;
                { c.is_proper_subset_of(c) } -> std::same_as<bool>;
                { c.intersects(c)          } -> std::same_as<bool>;
        }
;

// [template.bitset] over a storage of ours, which speaks the vocabulary and reads by block: what the storage has is forwarded, what it lacks is added through Traits. [design.md#owning-is-ours]
template<has_bitops Bits, bit_storage<Bits> Traits = bit_traits<Bits>>
        requires block_readable<Traits, Bits>
class bitset_adaptor : public detail::bits::allocator_typedef<Bits>
{
        // One wrapper, two counterparts it strictly extends: std::bitset at a static width, boost::dynamic_bitset at a run-time one. [design.md#a-strict-extension]
        static constexpr bool has_static_width = static_bit_extent<Traits, Bits>;

        // No iteration here by design, because neither counterpart has it: the two views refer into the storage instead. [design.md#views-over-owners]
        Bits m_bits{};

        template<std::input_iterator I>
        static constexpr bool block_iterator = std::same_as<std::remove_cvref_t<std::iter_value_t<I>>, typename Bits::block_type>;

        // The generic owner trait reaches the storage through owned_storage::bits. [design.md#an-owner-reads-as-its-storage]

        template<class> friend struct owned_storage;


        template<class B, ownership O, bit_storage<B> T>         friend class set_adaptor;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class sequence_adaptor;

        // The value through the trait: the blocks and the width. [design.md#the-hashing-invariant]
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bitset_adaptor const* v) noexcept
                -> void
        {
                detail::bits::hash_append_bits<Traits>(h, f, v->m_bits);
        }

public:
        // boost's typedefs; std::bitset has none, and a typedef changes no answer. The block is in the open again, boost's block interface being part of the extension. [design.md#a-strict-extension]
        using size_type  = std::size_t;
        using block_type = Bits::block_type;
        static constexpr std::size_t bits_per_block = Bits::bits_per_block;

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

                constexpr auto operator=(bool x) noexcept
                        -> reference&
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

                [[nodiscard]] constexpr auto operator~() const noexcept
                        -> bool
                {
                        return not Traits::at(m_ptr->m_bits, m_idx);
                }

                friend constexpr auto swap(reference x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }
                friend constexpr auto swap(reference x,     bool& y) noexcept -> void { bool const t = x; x = y; y = t; }
                friend constexpr auto swap(    bool& x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }

                constexpr auto flip() noexcept
                        -> reference&
                {
                        Traits::unchecked_assign(m_ptr->m_bits, m_idx, not Traits::at(m_ptr->m_bits, m_idx));
                        return *this;
                }
        };

        // boost's sentinel, which the searches answer at both widths.
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

        // boost's block-range constructor: the first block's low bit is position zero, and the width is a whole number of blocks.
        template<std::input_iterator I, std::sentinel_for<I> S>
        [[nodiscard]] constexpr bitset_adaptor(I first, S last)
                requires (not has_static_width) and block_iterator<I>
        {
                m_bits.append(first, last);
        }

        // boost's allocator arguments, where the storage takes one. [design.md#a-strict-extension]
        template<class Alloc>
                requires (not has_static_width) and std::same_as<Alloc, typename Bits::allocator_type>
        [[nodiscard]] constexpr explicit bitset_adaptor(Alloc const& alloc)
        :
                m_bits(alloc)
        {}

        template<class Alloc>
                requires (not has_static_width) and std::same_as<Alloc, typename Bits::allocator_type>
        [[nodiscard]] constexpr bitset_adaptor(std::size_t num_bits, unsigned long long val, Alloc const& alloc)
        :
                m_bits(num_bits, alloc)
        {
                from_ullong(val);
        }

        template<std::input_iterator I, std::sentinel_for<I> S, class Alloc>
                requires (not has_static_width) and block_iterator<I> and std::same_as<Alloc, typename Bits::allocator_type>
        [[nodiscard]] constexpr bitset_adaptor(I first, S last, Alloc const& alloc)
        :
                m_bits(alloc)
        {
                m_bits.append(first, last);
        }

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires requires (Bits const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        // Boost's, and so ours at both widths: the storage spells it alike, and an extension may add. [design.md#a-strict-extension]
        constexpr auto swap(bitset_adaptor& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
                requires std::swappable<Bits>
        {
                std::ranges::swap(m_bits, other.m_bits);
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

        // Constrained to the character types, so a pointer to a block reaches the block-range constructor above and never instantiates a string_view over the block. [design.md#a-strict-extension]
        template<class charT>
                requires (std::same_as<charT, char> or std::same_as<charT, wchar_t> or std::same_as<charT, char8_t> or std::same_as<charT, char16_t> or std::same_as<charT, char32_t>)
        [[nodiscard]] constexpr explicit bitset_adaptor(
                charT const* str,
                std::size_t n = std::basic_string_view<charT>::npos,
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

        // The counterparts' shifts are total and saturate to none; the storage's are unchecked, with pos < size() as their precondition, so the guard lives here. [design.md#the-one-guard]
        constexpr auto operator<<=(std::size_t pos) noexcept
                -> bitset_adaptor&
        {
                if (pos < size()) {
                        m_bits <<= pos;
                } else {
                        m_bits.reset();
                }
                return *this;
        }

        constexpr auto operator>>=(std::size_t pos) noexcept
                -> bitset_adaptor&
        {
                if (pos < size()) {
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

        // Element access: the one guard, then the unchecked write. It throws out_of_range at a static width as std::bitset does and asserts at a run-time one as boost does: the inconsistency is the counterparts' own. [design.md#the-one-guard]
        constexpr auto set(std::size_t pos, bool val = true)
                -> bitset_adaptor&
        {
                guard(pos);
                Traits::unchecked_assign(m_bits, pos, val);
                return *this;
        }

        constexpr auto reset(std::size_t pos)
                -> bitset_adaptor&
        {
                guard(pos);
                Traits::unchecked_assign(m_bits, pos, false);
                return *this;
        }

        constexpr auto flip(std::size_t pos)
                -> bitset_adaptor&
        {
                guard(pos);
                Traits::unchecked_assign(m_bits, pos, not Traits::at(m_bits, pos));
                return *this;
        }

        // boost's ranged forms, the one guard on the whole range, then the storage's own a word at a time. [design.md#the-one-guard]
        constexpr auto set(std::size_t pos, std::size_t len, bool val)
                -> bitset_adaptor&
        {
                guard_range(pos, len);
                m_bits.set(pos, len, val);
                return *this;
        }

        constexpr auto reset(std::size_t pos, std::size_t len)
                -> bitset_adaptor&
        {
                return set(pos, len, false);
        }

        constexpr auto flip(std::size_t pos, std::size_t len)
                -> bitset_adaptor&
        {
                guard_range(pos, len);
                m_bits.flip(pos, len);
                return *this;
        }

        // boost's test_set: the old value out, the new one in, behind the one guard.
        constexpr auto test_set(std::size_t pos, bool val = true)
                -> bool
        {
                guard(pos);
                auto const old = Traits::at(m_bits, pos);
                Traits::unchecked_assign(m_bits, pos, val);
                return old;
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

        // boost's at, throwing at both widths: an extension of std::bitset, which has none, and boost's own contract.
        [[nodiscard]] constexpr auto at(std::size_t pos) const
                -> bool
        {
                if (pos < size()) {
                        return Traits::at(m_bits, pos);
                }
                throw out_of_range(pos);
        }

        [[nodiscard]] constexpr auto at(std::size_t pos)
                -> reference
        {
                if (pos < size()) {
                        return { *this, pos };
                }
                throw out_of_range(pos);
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
        [[nodiscard]] constexpr auto count()      const noexcept -> std::size_t { return m_bits.count();      }
        [[nodiscard]] constexpr auto size()       const noexcept -> std::size_t { return m_bits.size();       }
        [[nodiscard]] constexpr auto num_blocks() const noexcept -> std::size_t { return m_bits.num_blocks(); }
        [[nodiscard]] constexpr auto max_size()   const noexcept -> std::size_t { return m_bits.max_size();   }

        [[nodiscard]] constexpr auto operator==(bitset_adaptor const& rhs) const noexcept -> bool = default;

        // The bit string's order, most significant position first, which is boost's: the storage's entry at equal widths, and boost's own walk over the top min(size()) positions with the shorter one first otherwise. [design.md#the-ordering-invariant]
        [[nodiscard]] friend constexpr auto operator<=>(bitset_adaptor const& lhs, bitset_adaptor const& rhs) noexcept
                -> std::strong_ordering
        {
                if constexpr (not has_static_width) {
                        if (lhs.size() != rhs.size()) {
                                return lhs.top_aligned_three_way(rhs);
                        }
                }
                return Traits::bitset_three_way(lhs.m_bits, rhs.m_bits);
        }

        [[nodiscard]] constexpr auto test(std::size_t pos) const
                -> bool
        {
                guard(pos);
                return Traits::at(m_bits, pos);
        }

        [[nodiscard]] constexpr auto all()  const noexcept -> bool { return m_bits.all();  }
        [[nodiscard]] constexpr auto any()  const noexcept -> bool { return m_bits.any();  }
        [[nodiscard]] constexpr auto none() const noexcept -> bool { return m_bits.none(); }

        // The set vocabulary boost has and std::bitset has not, at both widths: the storage spells it alike, and an extension may add. [design.md#a-strict-extension]
        constexpr auto operator-=(bitset_adaptor const& rhs) noexcept -> bitset_adaptor& { m_bits -= rhs.m_bits; return *this; }

        [[nodiscard]] constexpr auto is_subset_of       (bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.is_subset_of       (rhs.m_bits); }
        [[nodiscard]] constexpr auto is_proper_subset_of(bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.is_proper_subset_of(rhs.m_bits); }
        [[nodiscard]] constexpr auto intersects         (bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.intersects         (rhs.m_bits); }

        // boost's two searches and their mirror at both widths, npos where the total answer is the width; a zero width answers npos outright, its only answer. [design.md#degenerate-widths]
        [[nodiscard]] constexpr auto find_first() const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Traits>) {
                        return npos;
                } else {
                        auto const n = detail::bits::find_first<Traits>(m_bits);
                        return n == size() ? npos : n;
                }
        }

        [[nodiscard]] constexpr auto find_next(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Traits>) {
                        return npos;
                } else {
                        auto const n = detail::bits::find_next<Traits>(m_bits, pos);
                        return n == size() ? npos : n;
                }
        }

        // The reverse pair, ours: find_prev(pos) is the highest set position below pos, a pos past the width meaning from the end, so find_prev(npos) is find_last().
        // Total, so the generic walk rather than the trait's entry, whose contract is the iterator's cheaper one. [design.md#total-versus-precondition]
        [[nodiscard]] constexpr auto find_last() const noexcept
                -> std::size_t
        {
                return find_prev(size());
        }

        [[nodiscard]] constexpr auto find_prev(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Traits>) {
                        return npos;
                } else {
                        auto const n = detail::bits::scan_prev<Traits>(m_bits, pos);
                        return n == size() ? npos : n;
                }
        }

        // boost's block interface: every block out, including the clear tail, and at most every block in, the tail kept clear. [design.md#a-strict-extension]
        template<std::output_iterator<block_type> O>
        friend constexpr auto to_block_range(bitset_adaptor const& b, O result)
                -> void
        {
                for (auto const i : std::views::iota(0UZ, b.num_blocks())) {
                        *result++ = Traits::block(b.m_bits, i);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires block_iterator<I>
        friend constexpr auto from_block_range(I first, S last, bitset_adaptor& result)
                -> void
        {
                for (auto i = 0UZ; first != last; ++first, ++i) {
                        assert(i < result.num_blocks());
                        result.m_bits.set_block(i, *first);
                }
        }

        // Growth, boost's members, on storage that spells them alike: detected on the storage rather than reconciled by the trait. [design.md#growth]
        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
                requires (not has_static_width)
        {
                return size() == 0UZ;
        }

        constexpr auto resize(std::size_t num_bits, bool value = false)
                -> void
                requires requires (Bits& b) { b.resize(num_bits, value); }
        {
                m_bits.resize(num_bits, value);
        }

        constexpr auto clear()
                -> void
                requires requires (Bits& b) { b.clear(); }
        {
                m_bits.clear();
        }

        constexpr auto push_back(bool bit)
                -> void
                requires requires (Bits& b) { b.push_back(bit); }
        {
                m_bits.push_back(bit);
        }

        constexpr auto pop_back()
                -> void
                requires requires (Bits& b) { b.pop_back(); }
        {
                m_bits.pop_back();
        }

        template<class Block>
        constexpr auto append(Block value)
                -> void
                requires requires (Bits& b) { b.append(value); }
        {
                m_bits.append(value);
        }

        template<std::input_iterator I>
        constexpr auto append(I first, I last)
                -> void
                requires requires (Bits& b) { b.append(first, last); }
        {
                m_bits.append(first, last);
        }

        constexpr auto reserve(std::size_t num_bits)
                -> void
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

        constexpr auto shrink_to_fit()
                -> void
                requires requires (Bits& b) { b.shrink_to_fit(); }
        {
                m_bits.shrink_to_fit();
        }

private:
        // The one guard: out_of_range at a static width, std::bitset's, an assert at a run-time one, boost's. [design.md#the-one-guard]
        constexpr auto guard(std::size_t pos) const
                -> void
        {
                if constexpr (has_static_width) {
                        if (pos >= size()) {
                                throw out_of_range(pos);
                        }
                } else {
                        assert(pos < size());
                }
        }

        constexpr auto guard_range(std::size_t pos, std::size_t len) const
                -> void
        {
                if constexpr (has_static_width) {
                        if (pos + len > size()) {
                                throw out_of_range(pos + len);
                        }
                } else {
                        assert(pos + len <= size());
                }
        }

        // boost's unequal-width order, a word at a time: the top min(size()) positions of each paired from the top, read as words at either one's own alignment, then the shorter is less. [design.md#the-blit]
        // The top word of each window ends at its own width, so what it reads above the common length is the clear tail on both sides and needs no mask.
        [[nodiscard]] constexpr auto top_aligned_three_way(bitset_adaptor const& rhs) const noexcept
                -> std::strong_ordering
        {
                auto const m = std::ranges::min(size(), rhs.size());
                auto const lhs_start = size() - m;
                auto const rhs_start = rhs.size() - m;
                for (auto k = (m + bits_per_block - 1UZ) / bits_per_block; k-- != 0UZ;) {
                        auto const lhs_word = detail::bits::word_at<Traits>(m_bits, lhs_start + (k * bits_per_block));
                        auto const rhs_word = detail::bits::word_at<Traits>(rhs.m_bits, rhs_start + (k * bits_per_block));
                        if (auto const cmp = lhs_word <=> rhs_word; cmp != std::strong_ordering::equal) {
                                return cmp;
                        }
                }
                // The widths differ, which is how this walk was reached, so equal is not an answer here.
                return size() < rhs.size() ? std::strong_ordering::less : std::strong_ordering::greater;
        }

        constexpr auto from_ullong(unsigned long long val) noexcept
                -> void
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

// Boost has the free form beside the member; std::bitset has neither, and an extension may add. [design.md#a-strict-extension]
template<class Bits, class Traits>
constexpr auto swap(bitset_adaptor<Bits, Traits>& x, bitset_adaptor<Bits, Traits>& y) noexcept(noexcept(x.swap(y)))
        -> void
        requires std::swappable<Bits>
{
        x.swap(y);
}

// The owner's side of the view protocol: what a bit_set_view or bit_span over a bitset refers into. [design.md#views-over-owners]
template<class Bits, class Traits>
struct owned_storage<bitset_adaptor<Bits, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;

        // The storage itself, so one generic bit_traits can adapt every owner. [design.md#an-owner-reads-as-its-storage]
        [[nodiscard]] static constexpr auto bits(bitset_adaptor<Bits, Traits>& o) noexcept
                -> bits_type&
        {
                return o.m_bits;
        }

        [[nodiscard]] static constexpr auto bits(bitset_adaptor<Bits, Traits> const& o) noexcept
                -> bits_type const&
        {
                return o.m_bits;
        }
};

// A bitset reads exactly as the storage it wraps, so one specialization on bitset_adaptor adapts all three bitsets at
// once -- xstd::bitset<N>, xstd::inplace_bitset<N> and xstd::dynamic_bitset are aliases of it over a different
// contiguous_bit_container -- and gives them the direct view spelling, bit_set_view<xstd::bitset<N>> and bit_span<xstd::bitset<N>>.
// Every optional entry is relayed under its own guard, because absence is what the tiers select on: dropping num_blocks
// and block here would silently turn every word-parallel walk element-wise. [design.md#a-bitset-reads-as-its-storage]
template<class Bits, class Traits>
struct bit_traits<bitset_adaptor<Bits, Traits>>
{
        using bits_type = bitset_adaptor<Bits, Traits>;
        using inner     = owned_storage<bits_type>;

        static constexpr std::size_t extent = Traits::extent;

        // The three required entries.
        [[nodiscard]] static constexpr auto size(bits_type const& c) noexcept
                -> std::size_t
        {
                return Traits::size(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto at(bits_type const& c, std::size_t n) noexcept
                -> bool
        {
                return Traits::at(inner::bits(c), n);
        }

        // The optional ones, each relayed only where the storage's own trait has it.
        static constexpr auto unchecked_assign(bits_type& c, std::size_t n, bool value) noexcept
                -> void
                requires requires (Bits& b) { Traits::unchecked_assign(b, n, value); }
        {
                Traits::unchecked_assign(inner::bits(c), n, value);
        }

        static constexpr auto insert(bits_type& c, std::size_t n) noexcept(noexcept(Traits::insert(inner::bits(c), n)))
                -> void
                requires requires (Bits& b) { Traits::insert(b, n); }
        {
                Traits::insert(inner::bits(c), n);
        }

        static constexpr auto fill(bits_type& c, bool value) noexcept(noexcept(Traits::fill(inner::bits(c), value)))
                -> void
                requires requires (Bits& b) { Traits::fill(b, value); }
        {
                Traits::fill(inner::bits(c), value);
        }

        [[nodiscard]] static constexpr auto count(bits_type const& c) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::count(b); }
        {
                return Traits::count(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto all(bits_type const& c) noexcept
                -> bool
                requires requires (Bits const& b) { Traits::all(b); }
        {
                return Traits::all(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto any(bits_type const& c) noexcept
                -> bool
                requires requires (Bits const& b) { Traits::any(b); }
        {
                return Traits::any(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto none(bits_type const& c) noexcept
                -> bool
                requires requires (Bits const& b) { Traits::none(b); }
        {
                return Traits::none(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto num_blocks(bits_type const& c) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::num_blocks(b); }
        {
                return Traits::num_blocks(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto block(bits_type const& c, std::size_t i) noexcept
                requires requires (Bits const& b) { Traits::block(b, i); }
        {
                return Traits::block(inner::bits(c), i);
        }

        [[nodiscard]] static constexpr auto find_first(bits_type const& c) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::find_first(b); }
        {
                return Traits::find_first(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto find_last(bits_type const& c) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::find_last(b); }
        {
                return Traits::find_last(inner::bits(c));
        }

        [[nodiscard]] static constexpr auto find_next(bits_type const& c, std::size_t n) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::find_next(b, n); }
        {
                return Traits::find_next(inner::bits(c), n);
        }

        [[nodiscard]] static constexpr auto find_prev(bits_type const& c, std::size_t n) noexcept
                -> std::size_t
                requires requires (Bits const& b) { Traits::find_prev(b, n); }
        {
                return Traits::find_prev(inner::bits(c), n);
        }

        [[nodiscard]] static constexpr auto first_difference(bits_type const& x, bits_type const& y) noexcept
                requires requires (Bits const& b) { Traits::first_difference(b, b); }
        {
                return Traits::first_difference(inner::bits(x), inner::bits(y));
        }

        [[nodiscard]] static constexpr auto set_three_way(bits_type const& x, bits_type const& y) noexcept
                -> std::strong_ordering
                requires requires (Bits const& b) { Traits::set_three_way(b, b); }
        {
                return Traits::set_three_way(inner::bits(x), inner::bits(y));
        }

        [[nodiscard]] static constexpr auto sequence_three_way(bits_type const& x, bits_type const& y) noexcept
                -> std::strong_ordering
                requires requires (Bits const& b) { Traits::sequence_three_way(b, b); }
        {
                return Traits::sequence_three_way(inner::bits(x), inner::bits(y));
        }

        [[nodiscard]] static constexpr auto bitset_three_way(bits_type const& x, bits_type const& y) noexcept
                -> std::strong_ordering
                requires requires (Bits const& b) { Traits::bitset_three_way(b, b); }
        {
                return Traits::bitset_three_way(inner::bits(x), inner::bits(y));
        }
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
template<class Bits, class Traits> [[nodiscard]] constexpr auto operator-(bitset_adaptor<Bits, Traits> const& lhs, bitset_adaptor<Bits, Traits> const& rhs) noexcept(static_bit_extent<Traits, Bits>) -> bitset_adaptor<Bits, Traits> { auto nrv = lhs; nrv -= rhs; return nrv; }

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
