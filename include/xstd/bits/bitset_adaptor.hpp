//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BITSET_ADAPTOR_HPP
#define XSTD_BITS_BITSET_ADAPTOR_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <xstd/bits/detail/allocator_base_type.hpp> // allocator_base_type
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/hash.hpp>              // hash_append_bits, std_hash
#include <xstd/bits/detail/zero_width.hpp>        // zero_width
#include <xstd/bits/ownership.hpp>                // owned_storage, ownership, reading
#include <xstd/misc/concepts/specialization_of.hpp> // specialization_of_TN
#include <boost/hash2/hash_append.hpp>            // hash_append_tag
#include <algorithm>                              // min
#include <cassert>                                // assert
#include <compare>                                // strong_ordering
#include <concepts>                               // same_as, swappable
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
#include <span>                                   // dynamic_extent
#include <stdexcept>                              // invalid_argument, out_of_range, overflow_error
#include <string>                                 // basic_string, char_traits
#include <string_view>                            // basic_string_view
#include <type_traits>                            // is_nothrow_swappable_v, remove_cvref_t
#include <utility>                                // as_const

namespace xstd {

// [template.bitset] over a storage of ours, nominally: has_bitops used to ask structurally whether a storage spoke the bitset vocabulary, because a foreign one might. Only ours can be here now, and ours speaks it by construction, so the question was answering itself.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits>
class bitset_adaptor : public detail::bits::allocator_base_type<Bits>
{
        // One wrapper, two counterparts it strictly extends: std::bitset at a static width, boost::dynamic_bitset at a run-time one.
        static constexpr bool has_static_width = (Bits::extent != std::dynamic_extent);

        // No iteration here by design, because neither counterpart has it: the two views refer into the storage instead.
        Bits m_bits{};

        template<std::input_iterator I>
        static constexpr bool block_iterator = std::same_as<std::remove_cvref_t<std::iter_value_t<I>>, typename Bits::block_type>;

        // owned_storage names this owner's storage, so a view over a bitset is a view over what the bitset wraps.
        template<class> friend struct owned_storage;

        // Either reading's view refers into this owner's storage, and nothing else outside does: a bitset is committed to neither reading, which is what its two views are for.
        template<specialization_of_TN<detail::bits::contiguous_bit_container> B, ownership O>         friend class set_adaptor;
        template<specialization_of_TN<detail::bits::contiguous_bit_container> B, ownership O, bool W> friend class sequence_adaptor;

        // The value through the trait: the blocks and the width.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bitset_adaptor const* v) noexcept
                -> void
        {
                detail::bits::hash_append_bits(h, f, v->m_bits);
        }

public:
        // boost's typedefs; std::bitset has none, and a typedef changes no answer. The block is in the open again, boost's block interface being part of the extension.
        using size_type  = std::size_t;
        using block_type = Bits::block_type;
        static constexpr std::size_t bits_per_block = Bits::bits_per_block;

        // [bitset.refs], reaching the bits only through the unchecked way in; its own class, with the flip and ~ the sequence proxy lacks.
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

                // Assigns the bit, not the proxy: rebinding would break the swap below.
                constexpr auto operator=(reference const& x) noexcept -> reference&  // NOLINT(bugprone-unhandled-self-assignment)
                {
                        std::as_const(*this) = static_cast<bool>(x);
                        return *this;
                }

                // A proxy reference assigns through a const proxy, the shape the standard gives vector<bool>::reference.
                constexpr auto operator=(bool x) const noexcept -> reference const&  // NOLINT(misc-unconventional-assign-operator)
                {
                        m_ptr->m_bits.assign(m_idx, x);
                        return *this;
                }

                [[nodiscard]] constexpr explicit(false) operator bool() const noexcept  // NOLINT(misc-explicit-constructor)
                {
                        return m_ptr->m_bits.test(m_idx);
                }

                [[nodiscard]] constexpr auto operator~() const noexcept
                        -> bool
                {
                        return not m_ptr->m_bits.test(m_idx);
                }

                friend constexpr auto swap(reference x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }
                friend constexpr auto swap(reference x,     bool& y) noexcept -> void { bool const t = x; x = y; y = t; }
                friend constexpr auto swap(    bool& x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }

                constexpr auto flip() noexcept
                        -> reference&
                {
                        m_ptr->m_bits.assign(m_idx, not m_ptr->m_bits.test(m_idx));
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

        // A field of bits in, a field of bits out, in the bytes the three readings share -- and constrained on
        // container_source rather than on bit_castable, which is the one place this reading differs from the other
        // two. It ALREADY HAS the integer door, twice over, and admitting the integer family here would not widen
        // it but collide with it:
        //
        //   - the constructor above takes unsigned long long IMPLICITLY. A template admitting unsigned int would be
        //     an exact match where that one needs a conversion, so it would win for bitset<32> b(5u) -- and being
        //     explicit, it would make bitset<32> b = 5u ill-formed, which compiles today.
        //   - to_ullong() THROWS overflow_error where a set position lies beyond the word ([bitset.members]/34-37),
        //     where a byte copy would silently keep the low bits. Two contracts for one conversion is a trap, and
        //     the standard's is the one this reading owes.
        //
        // So integers keep their door and this opens the other one: std::bitset<N>, and any field of bits whose
        // layout bit_castable can prove.
        // NOT ITSELF, which the other two readings get for free and this one has to say. A templated constructor
        // is a candidate for copy-construction too, and this reading is the one whose own type the probe accepts:
        // it has set, count and size, so container_source runs the probe on it rather than declining early. The
        // copy constructor still wins on overload resolution, but the constraint is CHECKED first, and checking it
        // is what dragged a self-probe into every instantiation.
        template<class B>
                requires (not std::same_as<std::remove_cvref_t<B>, bitset_adaptor>) and Bits::template exchanges_bits_as_field<B>
        [[nodiscard]] constexpr explicit bitset_adaptor(B const& b) noexcept
        {
                m_bits.assign_bits(b);
        }

        template<class B>
                requires Bits::template exchanges_bits_as_field<B>
        [[nodiscard]] constexpr explicit operator B() const noexcept
        {
                return m_bits.template to_bits<B>();
        }

        // boost's block-range constructor: the first block's low bit is position zero, and the width is a whole number of blocks.
        template<std::input_iterator I, std::sentinel_for<I> S>
        [[nodiscard]] constexpr bitset_adaptor(I first, S last)
                requires (not has_static_width) and block_iterator<I>
        {
                m_bits.append(first, last);
        }

        // boost's allocator arguments, where the storage takes one.
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

        // Boost has the free form beside the member; std::bitset has neither, and an extension may add. Hidden rather than at namespace scope, unlike the operators below: xstd::swap(a, b) is a spelling people reach for by habit and a qualified operator is not, so here the hiding buys something.
        friend constexpr auto swap(bitset_adaptor& x, bitset_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
                requires std::swappable<Bits>
        {
                x.swap(y);
        }

        // Boost's, and so ours at both widths: the storage spells it alike, and an extension may add.
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
                                m_bits.assign(i, true);
                        } else {
                                throw invalid_argument(ch, zero, one);
                        }
                }
        }

        // Constrained to the character types, so a pointer to a block reaches the block-range constructor above and never instantiates a string_view over the block.
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

        // The counterparts' shifts are total and saturate to none; the storage's are unchecked, with pos < size() as their precondition, so the guard lives here.
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


        constexpr auto set  () noexcept -> bitset_adaptor& { m_bits.set  (); return *this; }
        constexpr auto reset() noexcept -> bitset_adaptor& { m_bits.reset(); return *this; }
        constexpr auto flip () noexcept -> bitset_adaptor& { m_bits.flip (); return *this; }

        // Element access: the one guard, then the unchecked write. It throws out_of_range at a static width as std::bitset does and asserts at a run-time one as boost does: the inconsistency is the counterparts' own. A zero width holds no position, so the guard throws for every pos and each of the five members below is, at that width, nothing but the throw. Said as its own arm rather than left after the guard, or the instantiation carries a tail no control flow reaches, which MSVC reports under /O2.
        constexpr auto set(std::size_t pos, [[maybe_unused]] bool val = true)
                -> bitset_adaptor&
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, val);
                        return *this;
                }
        }

        constexpr auto reset(std::size_t pos)
                -> bitset_adaptor&
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, false);
                        return *this;
                }
        }

        constexpr auto flip(std::size_t pos)
                -> bitset_adaptor&
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, not m_bits.test(pos));
                        return *this;
                }
        }

        // boost's ranged forms, the checked guard on the whole range at both widths, then the storage's own a word at a time.
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
        constexpr auto test_set(std::size_t pos, [[maybe_unused]] bool val = true)
                -> bool
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        auto const old = m_bits.test(pos);
                        m_bits.assign(pos, val);
                        return old;
                }
        }

        // The const subscript is unchecked on every counterpart, so it is test() unconditionally.
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

        // boost's at, throwing at both widths: an extension of std::bitset, which has none, and boost's own contract.
        [[nodiscard]] constexpr auto at(std::size_t pos) const
                -> bool
        {
                if (pos < size()) {
                        return m_bits.test(pos);
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
        [[nodiscard]] constexpr auto to_ulong()  const -> unsigned long      { return to_unsigned<unsigned long>();      }
        [[nodiscard]] constexpr auto to_ullong() const -> unsigned long long { return to_unsigned<unsigned long long>(); }

        template<
                class charT = char,
                class traits = std::char_traits<charT>,
                class Allocator = std::allocator<charT>
        >
        [[nodiscard]] constexpr auto to_string(charT zero = static_cast<charT>('0'), charT one = static_cast<charT>('1')) const
                -> std::basic_string<charT, traits, Allocator>
        {
                auto const N = size();
                // The finding is the zero-width instantiation, where empty is the answer.
                auto str = std::basic_string<charT, traits, Allocator>(N, zero);  // NOLINT(bugprone-string-constructor)
                for (auto const i : std::views::iota(0UZ, N)) {
                        if (m_bits.test(N - 1 - i)) {
                                str[i] = one;
                        }
                }
                return str;
        }

        // observers
        [[nodiscard]] constexpr auto count()      const noexcept -> std::size_t { return m_bits.count();      }
        [[nodiscard]] constexpr auto size()       const noexcept -> std::size_t { return m_bits.size();       }
        [[nodiscard]] constexpr auto num_blocks() const noexcept -> std::size_t { return m_bits.num_blocks(); }
        // boost's own answer and not the storage's own, the two differing by sixty-three positions at a run-time width: this reading is a strict extension of boost::dynamic_bitset, so an expression boost defines answers here what it answers there.
        [[nodiscard]] constexpr auto max_size()   const noexcept -> std::size_t { return m_bits.saturating_max_size(); }

        // A friend rather than the member std::bitset specifies: [class.compare.default]/1 admits either, and since P1185's reversed candidates the two accept the same mixed comparisons against the implicit unsigned long long. A namespace-scope template would not, deduction declining that conversion on both sides. Defaulted, the storage being the one member.
        [[nodiscard]] friend constexpr auto operator==(bitset_adaptor const& lhs, bitset_adaptor const& rhs) noexcept -> bool = default;

        // The bit string's order, most significant position first, which is boost's: the storage's entry at equal widths, and the top-aligned walk below otherwise. That walk is the one comparison here that cannot be blockwise -- two bit strings of different lengths is a question about N, not about the blocks -- so it lives with the reading that knows N.
        [[nodiscard]] friend constexpr auto operator<=>(bitset_adaptor const& lhs, bitset_adaptor const& rhs) noexcept
                -> std::strong_ordering
        {
                if constexpr (not has_static_width) {
                        if (lhs.size() != rhs.size()) {
                                return lhs.top_aligned_three_way(rhs);
                        }
                }
                return string_lexicographical_compare_three_way(lhs.m_bits, rhs.m_bits);
        }

        [[nodiscard]] constexpr auto test(std::size_t pos) const
                -> bool
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        return m_bits.test(pos);
                }
        }

        [[nodiscard]] constexpr auto all()  const noexcept -> bool { return m_bits.all();  }
        [[nodiscard]] constexpr auto any()  const noexcept -> bool { return m_bits.any();  }
        [[nodiscard]] constexpr auto none() const noexcept -> bool { return m_bits.none(); }

        // The set vocabulary boost has and std::bitset has not, at both widths: the storage spells it alike, and an extension may add.
        constexpr auto operator-=(bitset_adaptor const& rhs) noexcept -> bitset_adaptor& { m_bits -= rhs.m_bits; return *this; }

        [[nodiscard]] constexpr auto is_subset_of       (bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.is_subset_of       (rhs.m_bits); }
        [[nodiscard]] constexpr auto is_proper_subset_of(bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.is_proper_subset_of(rhs.m_bits); }
        [[nodiscard]] constexpr auto intersects         (bitset_adaptor const& rhs) const noexcept -> bool { return m_bits.intersects         (rhs.m_bits); }

        // The symmetric spelling beside boost's member, the pair swap and the storage both carry: a meets b exactly when b meets a. Forwarding this way and not the other, because a member of this name ends unqualified lookup before ADL begins, so the member can never reach the friend.
        [[nodiscard]] friend constexpr auto intersects(bitset_adaptor const& x, bitset_adaptor const& y) noexcept -> bool { return x.intersects(y); }

        // boost's two searches and their mirror at both widths, npos where the total answer is the width; a zero width answers npos outright, its only answer.
        [[nodiscard]] constexpr auto find_first() const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        return npos;
                } else {
                        auto const n = m_bits.find_first();
                        return n == size() ? npos : n;
                }
        }

        // Total, which is boost's own contract: a position at or past the width is one nothing can be set after, and npos is that answer rather than a precondition violation. The storage's step is not total -- it asserts is_valid(n) and steps to n + 1 -- so the guard is here, and it is the same guard the set reading's upper_bound already keeps over the same primitive. Without it find_next(npos) was the worst shape this can take: n + 1 wraps to zero, the scan starts from the beginning, and the answer is the FIRST set position.
        [[nodiscard]] constexpr auto find_next(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        return npos;
                } else {
                        if (pos >= size()) {
                                return npos;
                        }
                        auto const n = m_bits.exclusive_find_next(pos);
                        return n == size() ? npos : n;
                }
        }

        // The reverse pair, ours: find_prev(pos) is the highest set position below pos, a pos past the width meaning from the end, so find_prev(npos) is find_last().
        [[nodiscard]] constexpr auto find_last() const noexcept
                -> std::size_t
        {
                return find_prev(size());
        }

        [[nodiscard]] constexpr auto find_prev(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (detail::bits::zero_width<Bits>) {
                        return npos;
                } else {
                        // The storage's reverse step is a precondition rather than a total answer, and reverse iteration is what guards it there; here the guard is this: nothing is set below the first set position, which is the width where nothing is set at all.
                        auto const i = pos < size() ? pos : size();
                        return m_bits.find_first() >= i ? npos : m_bits.exclusive_find_prev(i);
                }
        }

        // boost's block interface: every block out, including the clear tail, and at most every block in, the tail kept clear.
        template<std::output_iterator<block_type> O>
        friend constexpr auto to_block_range(bitset_adaptor const& b, O result)
                -> void
        {
                for (auto const i : std::views::iota(0UZ, b.num_blocks())) {
                        *result++ = b.m_bits.block(i);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires block_iterator<I>
        friend constexpr auto from_block_range(I first, S last, bitset_adaptor& result)
                -> void
        {
                for (auto i = 0UZ; first != last; ++first, ++i) {
                        assert(i < result.num_blocks());
                        result.m_bits.block(i) = *first;
                }
                // Once, where a setter would have erased after every block.
                result.m_bits.erase_unused();
        }

        // Growth, boost's members, on storage that spells them alike: detected on the storage rather than reconciled by the trait.
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
        // The one guard: out_of_range at a static width, std::bitset's, an assert at a run-time one, boost's.
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

        // The same guard over a range, and unlike the one above it throws at both widths. That one's split is the counterparts' own: std::bitset::set(pos) throws and boost's asserts, so each of ours answers as its own counterpart does. These have no such pair to mirror -- std::bitset has no ranged form at all, the family being boost's alone -- so a static width had no counterpart to follow here and the throw was already ours to choose. Half a policy is not one, and this is the half to keep.
        //
        // Nor is it a narrowing of boost. The rule is that every expression *valid* on the counterpart is valid here with the same result ([a-strict-extension]), and a range past the width is not one: boost says so itself, in the BOOST_ASSERT that under NDEBUG leaves a masked write through a block index the blocks never allocated. Defining what boost leaves undefined is what an extension may add.
        //
        // Said as a subtraction rather than as pos + len, which wraps for a pos near the top of size_t: a wrapped sum is below every width, so the check the range was meant to fail is the one it would pass. It is pos and len the diagnostic names, the sum being the thing that is not a position.
        constexpr auto guard_range(std::size_t pos, std::size_t len) const
                -> void
        {
                if (pos > size() or len > size() - pos) {
                        throw out_of_range(pos, len);
                }
        }

        // boost's unequal-width order: the top min(size()) positions of each paired from the top, then the shorter is less. The shorter window always begins at 0, so only the longer can be out of step.
        [[nodiscard]] constexpr auto top_aligned_three_way(bitset_adaptor const& rhs) const noexcept
                -> std::strong_ordering
        {
                auto const m = std::ranges::min(size(), rhs.size());
                auto const lhs_start = size() - m;
                auto const rhs_start = rhs.size() - m;
                auto const nb = (m + bits_per_block - 1UZ) / bits_per_block;
                if ((lhs_start | rhs_start) % bits_per_block == 0) {
                        // The widths differ by a whole number of blocks, so the shared window is a block range on each side and the walk is blockwise: 25.1us to 9.9us over a million bits, which is what an equal-width comparison costs.
                        auto const li0 = lhs_start / bits_per_block;
                        auto const ri0 = rhs_start / bits_per_block;
                        for (auto k = nb; k-- != 0UZ;) {
                                if (auto const cmp = m_bits.block(li0 + k) <=> rhs.m_bits.block(ri0 + k); cmp != std::strong_ordering::equal) {
                                        return cmp;
                                }
                        }
                } else {
                        // Misaligned by a partial block, where one side's block straddles two of the other's: a funnel shift per step is the operation, not a shortfall of the walk.
                        for (auto k = nb; k-- != 0UZ;) {
                                auto const lhs_block = m_bits.block_at(lhs_start + (k * bits_per_block));
                                auto const rhs_block = rhs.m_bits.block_at(rhs_start + (k * bits_per_block));
                                if (auto const cmp = lhs_block <=> rhs_block; cmp != std::strong_ordering::equal) {
                                        return cmp;
                                }
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
                                m_bits.assign(i, true);
                        }
                }
        }

        // One position at a time over at most a word's worth, the overflow asked of the trait's search above the word.
        template<class Unsigned>
        [[nodiscard]] constexpr auto to_unsigned() const
                -> Unsigned
        {
                constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Unsigned>::digits);
                if (size() > digits and m_bits.exclusive_find_next(digits - 1UZ) != size()) {
                        throw overflow_error();
                }
                auto const M = std::ranges::min(size(), digits);
                auto nrv = Unsigned{0};
                for (auto const i : std::views::iota(0UZ, M)) {
                        if (m_bits.test(i)) {
                                nrv |= static_cast<Unsigned>(Unsigned{1} << i);
                        }
                }
                return nrv;
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

        // The ranged form's own, because the single-position message names pos against size() and a range can fail with a pos below it. The sum is prose here, not arithmetic: it is exactly the addition the guard refuses to make.
        [[nodiscard]] constexpr auto out_of_range(std::size_t pos, std::size_t len, std::source_location const& loc = std::source_location::current()) const
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: arguments ‘pos‘ and ‘len‘ are out of range [{} + {} > {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), pos, len, size()
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

// The owner's side of the view protocol: what a bit_set_view or bit_span over a bitset refers into.
template<class Bits>
struct owned_storage<bitset_adaptor<Bits>>
{
        using bits_type   = Bits;

        // Committed to neither reading, which is what its two views are for.
        static constexpr auto reads = reading::bitset;
};

}       // namespace xstd

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// bitset hash support [bitset.hash]; no redeclaration of std::hash's primary template, which [namespace.std] forbids.
template<class Bits>
struct hash<xstd::bitset_adaptor<Bits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bitset_adaptor<Bits> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

// NOLINTEND(bugprone-std-namespace-modification)

}       // namespace std

namespace xstd {

// bitset operators                                           [bitset.operators]
template<class Bits> [[nodiscard]] constexpr auto operator&(bitset_adaptor<Bits> const& lhs, bitset_adaptor<Bits> const& rhs) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits> [[nodiscard]] constexpr auto operator|(bitset_adaptor<Bits> const& lhs, bitset_adaptor<Bits> const& rhs) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits> [[nodiscard]] constexpr auto operator^(bitset_adaptor<Bits> const& lhs, bitset_adaptor<Bits> const& rhs) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv ^= rhs; return nrv; }
template<class Bits> [[nodiscard]] constexpr auto operator-(bitset_adaptor<Bits> const& lhs, bitset_adaptor<Bits> const& rhs) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv -= rhs; return nrv; }

// @= belongs to the left operand and @ does not, where std::bitset makes these three members. Templates rather than hidden friends: they reach nothing private, and nobody writes an operator qualified, so the hiding would buy nothing here -- unlike swap, whose qualified spelling is an accident people do make.
template<class Bits> [[nodiscard]] constexpr auto operator~(bitset_adaptor<Bits> const& lhs) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv.flip(); return nrv; }
template<class Bits> [[nodiscard]] constexpr auto operator<<(bitset_adaptor<Bits> const& lhs, std::size_t pos) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv <<= pos; return nrv; }
template<class Bits> [[nodiscard]] constexpr auto operator>>(bitset_adaptor<Bits> const& lhs, std::size_t pos) noexcept((Bits::extent != std::dynamic_extent)) -> bitset_adaptor<Bits> { auto nrv = lhs; nrv >>= pos; return nrv; }

// [bitset.operators]/6: up to N characters into a temporary string, then x = bitset(str), so a short read lands in the low bits as it does there; a run-time width reads every 0 or 1 on offer and is as wide as the characters read, as boost's is.
template<class charT, class traits, class Bits>
auto operator>>(std::basic_istream<charT, traits>& is, bitset_adaptor<Bits>& x)
        -> std::basic_istream<charT, traits>&
{
        auto const limit = [&] -> std::size_t {
                if constexpr ((Bits::extent != std::dynamic_extent)) {
                        return x.size();
                } else {
                        return std::numeric_limits<std::size_t>::max();
                }
        }();
        auto str = std::basic_string<charT, traits>();
        // Assigned inside an if constexpr the zero-width instantiation discards.
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
        x = bitset_adaptor<Bits>(str);
        if constexpr (not detail::bits::zero_width<Bits>) {
                if (str.empty()) {
                        state |= std::ios_base::failbit;
                        is.setstate(state);
                }
        }
        return is;
}

template<class charT, class traits, class Bits>
auto operator<<(std::basic_ostream<charT, traits>& os, bitset_adaptor<Bits> const& x)
        -> std::basic_ostream<charT, traits>&
{
        return os << x.template to_string<charT, traits, std::allocator<charT>>(
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('0'),
                std::use_facet<std::ctype<charT>>(os.getloc()).widen('1')
        );
}

}       // namespace xstd

#endif // XSTD_BITS_BITSET_ADAPTOR_HPP
