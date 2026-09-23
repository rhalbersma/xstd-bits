//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_BITSET_ADAPTOR_HPP
#define XSTD_BITS_DETAIL_BITSET_ADAPTOR_HPP

// Bitsets [bitset], Header <bitset> synopsis [bitset.syn]

#include <xstd/bits/detail/allocator_base_type.hpp>      // allocator_base_type
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/hash.hpp>                     // hash_append_bits, std_hash
#include <xstd/bits/detail/ownership.hpp>                // owned_storage, storage, window
#include <xstd/bits/detail/zero_width.hpp>               // zero_width
#include <xstd/bits/from_bits.hpp>                       // from_bits_t
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <boost/hash2/hash_append.hpp>                   // hash_append_tag
#include <algorithm>                                     // min, ranges::copy
#include <cassert>                                       // assert
#include <compare>                                       // strong_ordering
#include <concepts>                                      // integral, same_as, swappable
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint_least32_t
#include <format>                                        // format, formattable
#include <functional>                                    // hash
#include <ios>                                           // ios_base
#include <iosfwd>                                        // basic_istream, basic_ostream
#include <iterator>                                      // contiguous_iterator, input_iterator, iter_value_t, output_iterator, sentinel_for, sized_sentinel_for
#include <limits>                                        // numeric_limits
#include <locale>                                        // ctype, use_facet
#include <memory>                                        // allocator
#include <ranges>                                        // iota, swap
#include <source_location>                               // source_location
#include <span>                                          // dynamic_extent
#include <stdexcept>                                     // invalid_argument, out_of_range, overflow_error
#include <string>                                        // basic_string, char_traits
#include <string_view>                                   // basic_string_view
#include <type_traits>                                   // is_array_v, is_nothrow_swappable_v, is_standard_layout_v, is_trivially_copyable_v, is_trivially_default_constructible_v, remove_cv_t, remove_cvref_t
#include <utility>                                       // as_const, move

namespace xstd::bits::detail {

// [template.bitset] over a storage of ours, which speaks the bitset vocabulary by construction.
template<specialization_of_TN<contiguous_bit_container> Bits, class Derived = void>
class bitset_adaptor : public allocator_base_type<Bits>
{
        // One wrapper, two counterparts: std::bitset at a static width, boost::dynamic_bitset at a run-time one.
        static constexpr bool has_static_width = (Bits::extent != std::dynamic_extent);

        // No iteration here, neither counterpart having it: the two views refer into the storage instead.
        Bits m_bits{};

        template<std::input_iterator I>
        static constexpr bool block_iterator = std::same_as<std::remove_cvref_t<std::iter_value_t<I>>, typename Bits::block_type>;

        // owned_storage names this owner's storage, so a view over a bitset is a view over what the bitset wraps.
        template<class>
        friend struct owned_storage;

        // The container needs constraints only the vehicle can name; [class.friend]/3 ignores the void a view passes.
        friend Derived;

        // A view refers into this owner's storage, and only a reading that can view it is named.
        template<specialization_of_TN<contiguous_bit_container>, storage, class>
        friend class set_adaptor;
        template<specialization_of_TN<contiguous_bit_container>, storage, window, class, std::size_t>
        friend class sequence_adaptor;

        // The value through the trait: the blocks and the width.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, bitset_adaptor const* v) noexcept
                -> void
        {
                hash_append_bits(h, f, v->m_bits);
        }

        // The most-derived type is what every operation hands back, so the containers keep their own names.
        [[nodiscard]] constexpr auto self() noexcept
                -> Derived&
        {
                return static_cast<Derived&>(*this);
        }

public:
        // What a trait asks of this vehicle, every container built on it answering alike.
        using adaptor_type = bitset_adaptor;
        using bits_type = Bits;

        // boost's typedefs; std::bitset has none, and the block is in the open as boost's interface needs.
        using size_type = std::size_t;
        using block_type = Bits::block_type;
        static constexpr std::size_t bits_per_block = Bits::bits_per_block;

        // [bitset.refs], reaching the bits through the unchecked way in, with the flip and ~ the sequence proxy lacks.
        class reference
        {
                // A pointer, not a reference, so the copy constructor stays defaulted as [bitset.refs] declares it.
                bitset_adaptor* m_ptr{};
                std::size_t m_idx{};

                friend bitset_adaptor;

                [[nodiscard]] constexpr reference(bitset_adaptor& c, std::size_t idx) noexcept
                        : m_ptr(&c)
                        , m_idx(idx)
                {}

        public:
                reference(reference const& x) noexcept = default;
                ~reference() = default;

                constexpr auto operator=(bool x) noexcept
                        -> reference&
                {
                        std::as_const(*this) = x;
                        return *this;
                }

                // Assigns the bit, not the proxy: rebinding would break the swap below.
                constexpr auto operator=(reference const& x) noexcept -> reference& // NOLINT(bugprone-unhandled-self-assignment)
                {
                        std::as_const(*this) = static_cast<bool>(x);
                        return *this;
                }

                // A proxy reference assigns through a const proxy, as the standard gives vector<bool>::reference.
                constexpr auto operator=(bool x) const noexcept -> reference const& // NOLINT(misc-unconventional-assign-operator)
                {
                        m_ptr->m_bits.assign(m_idx, x);
                        return *this;
                }

                [[nodiscard]] constexpr explicit(false) operator bool() const noexcept // NOLINT(misc-explicit-constructor)
                {
                        return m_ptr->m_bits.test(m_idx);
                }

                [[nodiscard]] constexpr auto operator~() const noexcept
                        -> bool
                {
                        return not m_ptr->m_bits.test(m_idx);
                }

                friend constexpr auto swap(reference x, reference y) noexcept
                        -> void
                {
                        bool const t = x;
                        x = y;
                        y = t;
                }

                friend constexpr auto swap(reference x, bool& y) noexcept
                        -> void
                {
                        bool const t = x;
                        x = y;
                        y = t;
                }

                friend constexpr auto swap(bool& x, reference y) noexcept
                        -> void
                {
                        bool const t = x;
                        x = y;
                        y = t;
                }

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
        [[nodiscard]] bitset_adaptor() noexcept = default;

        // [bitset.cons]/2: the low bits of val, as many as the width admits; boost orders its two the other way.
        [[nodiscard]] constexpr explicit(false) bitset_adaptor(unsigned long long val) noexcept // NOLINT(misc-explicit-constructor)
                requires has_static_width
        {
                from_ullong(val);
        }

        [[nodiscard]] constexpr explicit bitset_adaptor(std::size_t num_bits, unsigned long long val = 0ULL)
                requires (not has_static_width)
                : m_bits(num_bits)
        {
                from_ullong(val);
        }

        // A field of bits in and out, constrained on container_source: the integer door is already taken.
        template<class B>
                requires (not std::same_as<std::remove_cvref_t<B>, bitset_adaptor>) and Bits::template
        exchanges_bits_as_field<B> [[nodiscard]] static constexpr auto from_bits(B const& b) noexcept
                -> Derived
        {
                auto result = Derived();
                result.m_bits.assign_bits(b);
                return result;
        }

        // Tagged as std::from_range is: any field the storage exchanges, integers wider than the ullong door included.
        template<class B>
                requires Bits::template
        exchanges_bits<B> [[nodiscard]] constexpr bitset_adaptor(xstd::from_bits_t, B const& b) noexcept
        {
                m_bits.assign_bits(b);
        }

        template<class B>
                requires Bits::template
        exchanges_bits_as_field<B> [[nodiscard]] constexpr auto to_bits() const noexcept
                -> B
        {
                return m_bits.template to_bits<B>();
        }

        // boost's block-range constructor: the first block's low bit is position zero.
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
                : m_bits(alloc)
        {}

        template<class Alloc>
                requires (not has_static_width) and std::same_as<Alloc, typename Bits::allocator_type>
        [[nodiscard]] constexpr bitset_adaptor(std::size_t num_bits, unsigned long long val, Alloc const& alloc)
                : m_bits(num_bits, alloc)
        {
                from_ullong(val);
        }

        template<std::input_iterator I, std::sentinel_for<I> S, class Alloc>
                requires (not has_static_width) and block_iterator<I> and std::same_as<Alloc, typename Bits::allocator_type>
        [[nodiscard]] constexpr bitset_adaptor(I first, S last, Alloc const& alloc)
                : m_bits(alloc)
        {
                m_bits.append(first, last);
        }

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires requires (Bits const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        // flat_set's door onto its representation, at a run-time width: the blocks come in and go out whole.
        using block_container_type = Bits::block_container_type;

        constexpr auto replace(block_container_type&& blocks) noexcept(noexcept(m_bits.replace(std::move(blocks))))
                -> void
                requires requires (Bits& b, block_container_type&& c) { b.replace(std::move(c)); }
        {
                m_bits.replace(std::move(blocks));
        }

        [[nodiscard]] constexpr auto extract() && noexcept(noexcept(std::move(m_bits).extract()))
                -> block_container_type
                requires requires (Bits&& b) { std::move(b).extract(); }
        {
                return std::move(m_bits).extract();
        }

        // Boost has the free form beside the member; hidden, since xstd::swap(a, b) is reached for by habit.
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
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        template<class charT, class traits, class Allocator>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                std::basic_string<charT, traits, Allocator> const& str,
                std::basic_string<charT, traits, Allocator>::size_type pos = 0,
                std::basic_string<charT, traits, Allocator>::size_type n = std::basic_string<charT, traits, Allocator>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : bitset_adaptor(std::basic_string_view<charT, traits>(str), pos, n, zero, one)
        {}

        template<class charT, class traits>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                std::basic_string_view<charT, traits> str,
                std::basic_string_view<charT, traits>::size_type pos = 0,
                std::basic_string_view<charT, traits>::size_type n = std::basic_string_view<charT, traits>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
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

        // LWG 4294's four char-like traits, plus one clause: a pointer to a block is the block range's argument.
        template<class charT>
                requires (not std::same_as<std::remove_cv_t<charT>, block_type>) and (not std::is_array_v<charT>) and std::is_trivially_copyable_v<charT> and std::is_standard_layout_v<charT> and std::is_trivially_default_constructible_v<charT>
        [[nodiscard]] constexpr explicit bitset_adaptor(
                charT const* str,
                std::size_t n = std::basic_string_view<charT>::npos,
                charT zero = static_cast<charT>('0'),
                charT one = static_cast<charT>('1')
        )
                : bitset_adaptor(n == std::basic_string_view<charT>::npos ? std::basic_string_view<charT>(str) : std::basic_string_view<charT>(str, n), 0, n, zero, one)
        {}

        // Members                                              [bitset.members]
        constexpr auto operator&=(bitset_adaptor const& rhs) noexcept
                -> Derived&
        {
                m_bits &= rhs.m_bits;
                return self();
        }

        constexpr auto operator|=(bitset_adaptor const& rhs) noexcept
                -> Derived&
        {
                m_bits |= rhs.m_bits;
                return self();
        }

        constexpr auto operator^=(bitset_adaptor const& rhs) noexcept
                -> Derived&
        {
                m_bits ^= rhs.m_bits;
                return self();
        }

        // The counterparts' shifts saturate to none; the storage's are unchecked, so the guard lives here.
        constexpr auto operator<<=(std::size_t pos) noexcept
                -> Derived&
        {
                if (pos < size()) {
                        m_bits <<= pos;
                } else {
                        m_bits.reset();
                }
                return self();
        }

        constexpr auto operator>>=(std::size_t pos) noexcept
                -> Derived&
        {
                if (pos < size()) {
                        m_bits >>= pos;
                } else {
                        m_bits.reset();
                }
                return self();
        }

        constexpr auto set() noexcept
                -> Derived&
        {
                m_bits.set();
                return self();
        }

        constexpr auto reset() noexcept
                -> Derived&
        {
                m_bits.reset();
                return self();
        }

        constexpr auto flip() noexcept
                -> Derived&
        {
                m_bits.flip();
                return self();
        }

        // Element access: the one guard, then the unchecked write. A zero width is its own arm, or MSVC sees dead code.
        constexpr auto set(std::size_t pos, [[maybe_unused]] bool val = true)
                -> Derived&
        {
                if constexpr (zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, val);
                        return self();
                }
        }

        constexpr auto reset(std::size_t pos)
                -> Derived&
        {
                if constexpr (zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, false);
                        return self();
                }
        }

        constexpr auto flip(std::size_t pos)
                -> Derived&
        {
                if constexpr (zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        m_bits.assign(pos, not m_bits.test(pos));
                        return self();
                }
        }

        // boost's ranged forms: the guard on the whole range, then the storage's own a word at a time.
        constexpr auto set(std::size_t pos, std::size_t len, bool val)
                -> Derived&
        {
                guard_range(pos, len);
                m_bits.set(pos, len, val);
                return self();
        }

        constexpr auto reset(std::size_t pos, std::size_t len)
                -> Derived&
        {
                return set(pos, len, false);
        }

        constexpr auto flip(std::size_t pos, std::size_t len)
                -> Derived&
        {
                guard_range(pos, len);
                m_bits.flip(pos, len);
                return self();
        }

        // boost's test_set: the old value out, the new one in, behind the one guard.
        constexpr auto test_set(std::size_t pos, [[maybe_unused]] bool val = true)
                -> bool
        {
                if constexpr (zero_width<Bits>) {
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
                return {*this, pos};
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
                        return {*this, pos};
                }
                throw out_of_range(pos);
        }

        // [bitset.members]/34-37: the value the bits spell, or overflow_error for a position beyond the word.
        [[nodiscard]] constexpr auto to_ulong() const
                -> unsigned long
        {
                return to_unsigned<unsigned long>();
        }

        [[nodiscard]] constexpr auto to_ullong() const
                -> unsigned long long
        {
                return to_unsigned<unsigned long long>();
        }

        template<
                class charT = char,
                class traits = std::char_traits<charT>,
                class Allocator = std::allocator<charT>>
        [[nodiscard]] constexpr auto to_string(charT zero = static_cast<charT>('0'), charT one = static_cast<charT>('1')) const
                -> std::basic_string<charT, traits, Allocator>
        {
                auto const N = size();
                // The finding is the zero-width instantiation, where empty is the answer.
                auto str = std::basic_string<charT, traits, Allocator>(N, zero); // NOLINT(bugprone-string-constructor)
                for (auto const i : std::views::iota(0UZ, N)) {
                        if (m_bits.test(N - 1 - i)) {
                                str[i] = one;
                        }
                }
                return str;
        }

        // observers
        [[nodiscard]] constexpr auto count() const noexcept
                -> std::size_t
        {
                return m_bits.count();
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> std::size_t
        {
                return m_bits.size();
        }

        [[nodiscard]] constexpr auto num_blocks() const noexcept
                -> std::size_t
        {
                return m_bits.num_blocks();
        }

        // boost's answer and not the storage's, the two differing by sixty-three positions at a run-time width.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> std::size_t
        {
                return m_bits.saturating_max_size();
        }

        // A friend rather than the member std::bitset specifies: [class.compare.default]/1 admits either.
        [[nodiscard]] friend auto operator==(bitset_adaptor const& lhs, bitset_adaptor const& rhs) noexcept -> bool = default;

        // The bit string's order, most significant position first: two lengths is a question about N, not blocks.
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
                if constexpr (zero_width<Bits>) {
                        throw out_of_range(pos);
                } else {
                        guard(pos);
                        return m_bits.test(pos);
                }
        }

        [[nodiscard]] constexpr auto all() const noexcept
                -> bool
        {
                return m_bits.all();
        }

        [[nodiscard]] constexpr auto any() const noexcept
                -> bool
        {
                return m_bits.any();
        }

        [[nodiscard]] constexpr auto none() const noexcept
                -> bool
        {
                return m_bits.none();
        }

        // The set vocabulary boost has and std::bitset has not, which the storage spells alike at both widths.
        constexpr auto operator-=(bitset_adaptor const& rhs) noexcept
                -> Derived&
        {
                m_bits -= rhs.m_bits;
                return self();
        }

        [[nodiscard]] constexpr auto is_subset_of(bitset_adaptor const& rhs) const noexcept
                -> bool
        {
                return m_bits.is_subset_of(rhs.m_bits);
        }

        [[nodiscard]] constexpr auto is_proper_subset_of(bitset_adaptor const& rhs) const noexcept
                -> bool
        {
                return m_bits.is_proper_subset_of(rhs.m_bits);
        }

        [[nodiscard]] constexpr auto intersects(bitset_adaptor const& rhs) const noexcept
                -> bool
        {
                return m_bits.intersects(rhs.m_bits);
        }

        // The symmetric spelling beside boost's member: a member of this name would end lookup before ADL.
        [[nodiscard]] friend constexpr auto intersects(bitset_adaptor const& x, bitset_adaptor const& y) noexcept
                -> bool
        {
                return x.intersects(y);
        }

        // boost's two searches and their mirror, npos where the total answer is the width.
        [[nodiscard]] constexpr auto find_first() const noexcept
                -> std::size_t
        {
                if constexpr (zero_width<Bits>) {
                        return npos;
                } else {
                        auto const n = m_bits.find_first();
                        return n == size() ? npos : n;
                }
        }

        // Total, as boost's contract is: the storage's step asserts instead, so the guard is at this reading.
        [[nodiscard]] constexpr auto find_next(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (zero_width<Bits>) {
                        return npos;
                } else {
                        if (pos >= size()) {
                                return npos;
                        }
                        auto const n = m_bits.exclusive_find_next(pos);
                        return n == size() ? npos : n;
                }
        }

        // The reverse pair: find_prev(pos) is the highest set position below pos, so find_prev(npos) is find_last().
        [[nodiscard]] constexpr auto find_last() const noexcept
                -> std::size_t
        {
                return find_prev(size());
        }

        [[nodiscard]] constexpr auto find_prev(std::size_t pos) const noexcept
                -> std::size_t
        {
                if constexpr (zero_width<Bits>) {
                        return npos;
                } else {
                        // Nothing is set below the first set position, which is the width where nothing is set.
                        auto const i = pos < size() ? pos : size();
                        return m_bits.find_first() >= i ? npos : m_bits.exclusive_find_prev(i);
                }
        }

        // boost's block interface: every block out including the clear tail, every block in with the tail kept clear.
        template<std::output_iterator<block_type> O>
        friend constexpr auto to_block_range(bitset_adaptor const& b, O result)
                -> void
        {
                if constexpr (std::contiguous_iterator<O>) {
                        std::ranges::copy(b.m_bits.blocks(), result);
                } else {
                        for (auto const i : std::views::iota(0UZ, b.num_blocks())) {
                                *result++ = b.m_bits.block(i);
                        }
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires block_iterator<I>
        friend constexpr auto from_block_range(I first, S last, bitset_adaptor& result)
                -> void
        {
                // A sized sentinel too: the bulk copy must know the length before it writes.
                if constexpr (std::contiguous_iterator<I> and std::sized_sentinel_for<S, I>) {
                        // Spelled inside the assert: named, it is unread in a Release build, which is C4189.
                        assert(static_cast<std::size_t>(last - first) <= result.num_blocks());
                        std::ranges::copy(first, last, result.m_bits.blocks().begin());
                } else {
                        for (auto i = 0UZ; first != last; ++first, ++i) {
                                assert(i < result.num_blocks());
                                result.m_bits.block(i) = *first;
                        }
                }
                // Once, where a setter would have erased after every block.
                result.m_bits.erase_unused();
        }

        // Growth, boost's members, detected on the storage rather than reconciled by a trait.
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

        // The same guard over a range: a run-time width asserts as boost does, a static one throws as its own set does.
        constexpr auto guard_range(std::size_t pos, std::size_t len) const
                -> void
        {
                if constexpr (has_static_width) {
                        if (pos > size() or len > size() - pos) {
                                throw out_of_range(pos, len);
                        }
                } else {
                        assert(pos <= size() and len <= size() - pos);
                }
        }

        // boost's unequal-width order: the top min(size()) positions paired from the top, then the shorter is less.
        [[nodiscard]] constexpr auto top_aligned_three_way(bitset_adaptor const& rhs) const noexcept
                -> std::strong_ordering
        {
                auto const m = std::ranges::min(size(), rhs.size());
                auto const lhs_start = size() - m;
                auto const rhs_start = rhs.size() - m;
                auto const nb = (m + bits_per_block - 1UZ) / bits_per_block;
                if ((lhs_start | rhs_start) % bits_per_block == 0) {
                        // Widths differing by whole blocks make the shared window a block range, walked blockwise.
                        auto const li0 = lhs_start / bits_per_block;
                        auto const ri0 = rhs_start / bits_per_block;
                        for (auto k = nb; k-- != 0UZ;) {
                                if (auto const cmp = m_bits.block(li0 + k) <=> rhs.m_bits.block(ri0 + k); cmp != std::strong_ordering::equal) {
                                        return cmp;
                                }
                        }
                } else {
                        // Misaligned by a partial block: a funnel shift per step is the operation, not a shortfall.
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

        // Three arms by what the character can be written down as: only char has a narrow formatter.
        template<class charT>
        static constexpr auto invalid_argument(
                charT ch, charT zero = static_cast<charT>('0'), charT one = static_cast<charT>('1'),
                std::source_location const& loc = std::source_location::current()
        )
        {
                // The format string is spelled per arm: std::format takes a format_string, which is consteval.
                if constexpr (std::formattable<charT, char>) {
                        return std::invalid_argument(
                                std::format(
                                        "{}:{}:{}: exception: ‘{}‘: invalid argument ‘ch‘ [{} != {} or {}]",
                                        loc.file_name(), loc.line(), loc.column(), loc.function_name(), ch, zero, one
                                )
                        );
                } else if constexpr (std::integral<charT>) {
                        // A code unit is a number where it is not a character, which is every char type but char.
                        return std::invalid_argument(
                                std::format(
                                        "{}:{}:{}: exception: ‘{}‘: invalid argument ‘ch‘ [{} != {} or {}]",
                                        loc.file_name(), loc.line(), loc.column(), loc.function_name(),
                                        // On one line, or gcov counts two the call never reaches.
                                        static_cast<std::uint_least32_t>(ch), static_cast<std::uint_least32_t>(zero), static_cast<std::uint_least32_t>(one)
                                )
                        );
                } else {
                        // Char-like, and neither a character nor a number to anything that could write it down.
                        return std::invalid_argument(
                                std::format(
                                        "{}:{}:{}: exception: ‘{}‘: invalid argument ‘ch‘",
                                        loc.file_name(), loc.line(), loc.column(), loc.function_name()
                                )
                        );
                }
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

        // The ranged form's own: the single-position message names pos against size(), and a range can fail below it.
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

// Every container built on the vehicle, the vehicle itself being nobody's owner.
template<class T>
concept owning_bitset_adaptor = requires { typename T::bits_type; } and std::derived_from<T, bitset_adaptor<typename T::bits_type, T>>;

// The owner's side of the view protocol: what a bit_set_view or bit_span over a bitset refers into.
template<class Bits, class Derived>
struct owned_storage<bitset_adaptor<Bits, Derived>>
{
        using bits_type = Bits;

        // Committed to neither reading, which is what its two views are for.
        static constexpr auto reads = reading::bitset;
};

} // namespace xstd::bits::detail

namespace std {

// NOLINTBEGIN(bugprone-std-namespace-modification)

// bitset hash support [bitset.hash]; no redeclaration of std::hash's primary template, which [namespace.std] forbids.
template<class Bits, class Derived>
struct hash<xstd::bits::detail::bitset_adaptor<Bits, Derived>>
{
        [[nodiscard]] constexpr auto operator()(xstd::bits::detail::bitset_adaptor<Bits, Derived> const& v) const noexcept
                -> std::size_t
        {
                return xstd::bits::detail::std_hash(v);
        }
};

// NOLINTEND(bugprone-std-namespace-modification)

} // namespace std

namespace xstd::bits::detail {

// bitset operators                                           [bitset.operators]
template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator&(bitset_adaptor<Bits, Derived> const& lhs, bitset_adaptor<Bits, Derived> const& rhs) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv &= rhs;
        return nrv;
}

template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator|(bitset_adaptor<Bits, Derived> const& lhs, bitset_adaptor<Bits, Derived> const& rhs) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv |= rhs;
        return nrv;
}

template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator^(bitset_adaptor<Bits, Derived> const& lhs, bitset_adaptor<Bits, Derived> const& rhs) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv ^= rhs;
        return nrv;
}

template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator-(bitset_adaptor<Bits, Derived> const& lhs, bitset_adaptor<Bits, Derived> const& rhs) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv -= rhs;
        return nrv;
}

// @= belongs to the left operand and @ does not, where std::bitset makes these three members.
template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator~(bitset_adaptor<Bits, Derived> const& lhs) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv.flip();
        return nrv;
}

template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator<<(bitset_adaptor<Bits, Derived> const& lhs, std::size_t pos) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv <<= pos;
        return nrv;
}

template<class Bits, class Derived>
[[nodiscard]] constexpr auto operator>>(bitset_adaptor<Bits, Derived> const& lhs, std::size_t pos) noexcept((Bits::extent != std::dynamic_extent))
        -> Derived
{
        auto nrv = static_cast<Derived const&>(lhs);
        nrv >>= pos;
        return nrv;
}

// [bitset.operators]/6: up to N characters into a temporary string, then x = bitset(str), a short read landing low.
template<class charT, class traits, class Bits, class Derived>
auto operator>>(std::basic_istream<charT, traits>& is, bitset_adaptor<Bits, Derived>& x)
        -> std::basic_istream<charT, traits>&
{
        auto const limit = [&] -> std::size_t {
                if constexpr ((Bits::extent != std::dynamic_extent)) {
                        return x.size();
                } else {
                        return std::numeric_limits<std::size_t>::max();
                }
        }();
        // [bitset.operators]/4 makes this a formatted input function, and the sentry is what that means.
        auto const guard = typename std::basic_istream<charT, traits>::sentry(is);
        if (not guard) {
                // A failed sentry extracts nothing, so x keeps the value it had ([istream.formatted.reqmts]).
                return is;
        }
        auto str = std::basic_string<charT, traits>();
        // Assigned inside an if constexpr the zero-width instantiation discards.
        auto state = std::ios_base::goodbit; // NOLINT(misc-const-correctness)
        charT ch;
        // One peek per character: peeking twice sets eofbit then failbit, failing a short but valid read.
        while (str.size() < limit) {
                auto const next = is.peek();
                if (not traits::eq_int_type(next, is.widen('0')) and not traits::eq_int_type(next, is.widen('1'))) {
                        break;
                }
                is >> ch;
                str.push_back(ch);
        }
        x = Derived(str);
        if constexpr (not bits::detail::zero_width<Bits>) {
                if (str.empty()) {
                        state |= std::ios_base::failbit;
                        is.setstate(state);
                }
        }
        return is;
}

template<class charT, class traits, class Bits, class Derived>
auto operator<<(std::basic_ostream<charT, traits>& os, bitset_adaptor<Bits, Derived> const& x)
        -> std::basic_ostream<charT, traits>&
{
        return os << x.template to_string<charT, traits, std::allocator<charT>>(
                       std::use_facet<std::ctype<charT>>(os.getloc()).widen('0'),
                       std::use_facet<std::ctype<charT>>(os.getloc()).widen('1')
               );
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_BITSET_ADAPTOR_HPP
