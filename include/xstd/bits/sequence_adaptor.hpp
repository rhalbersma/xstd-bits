//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SEQUENCE_ADAPTOR_HPP
#define XSTD_BITS_SEQUENCE_ADAPTOR_HPP

#include <xstd/bits/bit_traits.hpp>               // all, any, bit_storage, bit_traits, count, none, static_bit_extent, word_at
#include <xstd/bits/detail/allocator_base_type.hpp> // allocator_base_type
#include <xstd/bits/detail/hash.hpp>              // hash_append_bits, std_hash
#include <xstd/bits/detail/intrin.hpp>            // countr_zero, popcount
#include <xstd/bits/detail/shift.hpp>             // shl, shr
#include <xstd/bits/detail/random_access.hpp>     // random_access_bit_iterator, random_access_bit_reference
#include <xstd/bits/ownership.hpp>                // owned_bits_t, owned_storage, owned_traits_t, owner_of, ownership, owns
#include <xstd/misc/type_traits/empty_base_type.hpp>          // empty_base_type
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/hash2/hash_append.hpp>            // hash_append_tag
#include <algorithm>                              // copy, min, remove_if
#include <cassert>                                // assert
#include <compare>                                // strong_ordering
#include <concepts>                               // constructible_from, convertible_to, invocable, same_as, swap, swappable
#include <cstddef>                                // ptrdiff_t, size_t
#include <format>                                 // format
#include <functional>                             // hash
#include <initializer_list>                       // initializer_list
#include <iterator>                               // input_iterator, make_reverse_iterator, reverse_iterator, sentinel_for
#include <limits>                                 // numeric_limits
#include <ranges>                                 // begin, enable_borrowed_range, enable_view, end, from_range_t, input_range, range_reference_t, size, sized_range, subrange
#include <source_location>                        // source_location
#include <span>                                   // dynamic_extent
#include <stdexcept>                              // out_of_range
#include <type_traits>                            // conditional_t, false_type, is_invocable_r_v, is_nothrow_swappable_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                                // as_const, declval, forward, move, pair

// The sequence reading, [array] over any Bits with a bit_traits specialization, owning it or referring to it. [design.md#the-three-adaptors]
namespace xstd {

namespace detail::sequence {

// The bits a window's word holds: every one but for the last word, which holds what is left over. [design.md#windows]
template<class Block>
[[nodiscard]] constexpr auto word_mask(std::size_t count) noexcept
        -> Block
{
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Block>::digits);
        // No shift by digits, which is undefined: a full word is every bit, spelled without one.
        return count == digits ? static_cast<Block>(~Block{}) : static_cast<Block>(detail::bits::shl(Block{1}, count) - Block{1});
}

// Continue unless the functor says otherwise: a void functor always continues, a bool one says. [design.md#the-sequence-for-each] [design.md#the-functor-takes-a-value]
template<class F>
[[nodiscard]] constexpr auto invoke_continues(F& f, bool value)
        -> bool
{
        if constexpr (std::is_invocable_r_v<bool, F&, bool>) {
                return f(auto(value));
        } else {
                f(auto(value));
                return true;
        }
}

// One tier each, because the tier is the seam and sharing a body puts the whole over readability-function-cognitive-complexity's threshold. [design.md#one-function-per-tier]

// Every position, lowest first, a word at a time: the outer loop loads once per word and the reload becomes the inner loop's exit test, which a flat operator++ can never express. [design.md#the-sequence-for-each]
template<class Traits, class Bits, class F>
constexpr auto walk_words(Bits const& c, std::size_t offset, std::size_t size, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const count = std::ranges::min(digits, size - k);
                auto const word = detail::bits::word_at<Traits>(c, offset + k);
                for (auto n = 0UZ; n < count; ++n) {
                        if (not invoke_continues(f, (detail::bits::shr(word, n) & block_type{1}) != block_type{})) {
                                return;
                        }
                }
        }
}

// The other tier: a storage with no block access -- libc++'s std::bitset and boost's are the ones -- walks positions, which is what the iterator does and is still the same answer. [design.md#windows]
template<class Traits, class Bits, class F>
constexpr auto walk_positions(Bits const& c, std::size_t offset, std::size_t size, F& f)
        -> void
{
        for (auto n = 0UZ; n < size; ++n) {
                if (not invoke_continues(f, Traits::at(c, offset + n))) {
                        return;
                }
        }
}

// The three aggregates over a window, each masked to what the window holds: a word at a time, so a window pays what the whole pays and not a test per bit. [design.md#windows]
template<class Traits, class Bits>
[[nodiscard]] constexpr auto count_words(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> std::size_t
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        auto n = 0UZ;
        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = word_mask<block_type>(std::ranges::min(digits, size - k));
                n += detail::bits::popcount(static_cast<block_type>(detail::bits::word_at<Traits>(c, offset + k) & mask));
        }
        return n;
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto any_words(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> bool
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = word_mask<block_type>(std::ranges::min(digits, size - k));
                if (static_cast<block_type>(detail::bits::word_at<Traits>(c, offset + k) & mask) != block_type{}) {
                        return true;
                }
        }
        return false;
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto all_words(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> bool
{
        using block_type = std::remove_cvref_t<decltype(Traits::block(c, 0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = word_mask<block_type>(std::ranges::min(digits, size - k));
                if (static_cast<block_type>(detail::bits::word_at<Traits>(c, offset + k) & mask) != mask) {
                        return false;
                }
        }
        return true;
}

// The same three over a window of anything else, one position at a time, which is what that storage offers.
template<class Traits, class Bits>
[[nodiscard]] constexpr auto count_positions(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> std::size_t
{
        auto n = 0UZ;
        for (auto k = 0UZ; k < size; ++k) {
                n += Traits::at(c, offset + k) ? 1UZ : 0UZ;
        }
        return n;
}

template<class Traits, class Bits>
[[nodiscard]] constexpr auto any_positions(Bits const& c, std::size_t offset, std::size_t size, bool value) noexcept
        -> bool
{
        for (auto k = 0UZ; k < size; ++k) {
                if (Traits::at(c, offset + k) == value) {
                        return true;
                }
        }
        return false;
}

}       // namespace detail::sequence

// A sequence adaptor of any shape, told by its public typedefs, whose trait reads blocks of the given type: what a blit reads. [design.md#the-blit]
template<class S, class Block>
concept blit_source =
        requires { typename S::subspan_type; typename S::traits_type; typename S::traits_type::bits_type; } and
        requires (S::traits_type::bits_type const& c, std::size_t i) {
                { S::traits_type::block(c, i) } -> std::same_as<Block>;
                { S::traits_type::num_blocks(c) } -> std::convertible_to<std::size_t>;
        };

// An owner names its storage's allocator, as std::vector<bool> names its own; a view names none, owning nothing. [design.md#the-sequence-contract]
template<class Bits, ownership Own, bool Windowed, bit_storage<Bits> Traits = bit_traits<std::remove_const_t<Bits>>>
class sequence_adaptor : public std::conditional_t<owns(Own), detail::bits::allocator_base_type<std::remove_const_t<Bits>>, xstd::empty_base_type<>>
{
        static constexpr bool is_owner  = owns(Own);
        static constexpr bool is_window = Windowed;
        static_assert(not (is_owner and is_window), "a window views what another owns");

        using bits_type = std::remove_const_t<Bits>;

        // Growth is the owner's over storage that grows: a view must never resize what it does not own. [design.md#growth]
        static constexpr bool can_grow = is_owner and not static_bit_extent<Traits, bits_type> and requires (bits_type& b, std::size_t n, bool value) { b.resize(n, value); b.push_back(value); b.pop_back(); b.clear(); };

        // A window is what std::span stores, the pointer's role split over a pointer and a position because bits are not addressable: the iterator's two fields and a size. [design.md#windows]
        struct window
        {
                Bits* ptr;
                std::size_t offset;
                std::size_t size;
        };

        std::conditional_t<is_owner, Bits, std::conditional_t<is_window, window, Bits*>> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow, a window's pointer likewise. [design.md#ownership-is-not-an-axis]
        [[nodiscard]] constexpr auto storage(this auto&& self) noexcept
                -> auto&&
        {
                if constexpr (is_owner) {
                        return self.m_bits;
                } else if constexpr (is_window) {
                        return *self.m_bits.ptr;
                } else {
                        return *self.m_bits;
                }
        }

        // Where this sequence starts in the storage: zero but for a window, so every position below is offset once, here. [design.md#windows]
        [[nodiscard]] constexpr auto offset() const noexcept
                -> std::size_t
        {
                if constexpr (is_window) {
                        return m_bits.offset;
                } else {
                        return 0UZ;
                }
        }

        // The window's constructor, which first, last and subspan call and nothing else does.
        [[nodiscard]] constexpr sequence_adaptor(Bits* ptr, std::size_t offset, std::size_t size) noexcept
                requires is_window
        :
                m_bits{ ptr, offset, size }
        {}

        // What the accessor hands a given self, const included: the iterator and the proxy are spelled over exactly that.
        template<class Self>
        using storage_t = std::remove_reference_t<decltype(std::declval<Self>().storage())>;

        template<class Self> using iterator_t  = detail::bits::random_access_bit_iterator <storage_t<Self>, Traits>;
        template<class Self> using reference_t = detail::bits::random_access_bit_reference<storage_t<Self>, Traits>;

        // A source the blit can read by block, of this storage's own block type. [design.md#the-blit]
        template<class S>
        static constexpr bool blittable = blit_source<S, typename bits_type::block_type>;

        // A storage that takes a masked word at any position: ours, which is what a window's bulk operators write through.
        static constexpr bool word_writable = requires (bits_type& b, std::size_t pos, bits_type::block_type w) { b.set_word(pos, w, w); };

        // Either reading's view refers into this owner's storage, and nothing else outside does. [design.md#views-over-owners]
        template<class B, ownership O, bit_storage<B> T>         friend class set_adaptor;
        template<class B, ownership O, bool W, bit_storage<B> T> friend class sequence_adaptor;

        // The value under the sequence reading, the owner's alone as == is: a view follows span and hashes no more than it compares. [design.md#the-hashing-invariant]
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, sequence_adaptor const* v) noexcept
                -> void
                requires is_owner
        {
                detail::bits::hash_append_bits<Traits>(h, f, v->storage());
        }

public:
        // types
        using value_type             = bool;
        using traits_type            = Traits;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = detail::bits::random_access_bit_reference<Bits, Traits>;
        using const_reference        = detail::bits::random_access_bit_reference<Bits const, Traits>;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = detail::bits::random_access_bit_iterator<Bits, Traits>;
        using const_iterator         = detail::bits::random_access_bit_iterator<Bits const, Traits>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy; an owner is built the way std::array is, or std::vector where the storage grows, a view only from what it views.
        [[nodiscard]] constexpr sequence_adaptor() noexcept requires is_owner = default;

        [[nodiscard]] constexpr explicit sequence_adaptor(size_type n)
                requires can_grow
        :
                m_bits(n)
        {}

        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value)
                requires can_grow
        :
                m_bits(n)
        {
                if (value) {
                        Traits::fill(m_bits, true);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>>
        [[nodiscard]] constexpr sequence_adaptor(I first, S last)
        {
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        [[nodiscard]] constexpr sequence_adaptor(std::from_range_t, R&& rg)
        :
                sequence_adaptor(std::ranges::begin(rg), std::ranges::end(rg))
        {}

        [[nodiscard]] constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires can_grow
        :
                sequence_adaptor(il.begin(), il.end())
        {}

        // std::array's aggregate initialization, as a constructor: what is listed leads and the rest stays false, a longer list being the error it is there. [design.md#the-sequence-contract]
        constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires is_owner and (not can_grow)
        :
                m_bits()
        {
                assert(il.size() <= size());
                std::ranges::copy(il, begin());
        }

        // [vector.bool]'s allocator arguments, where the storage takes one: deduced and matched, so a storage without one has no such constructor. [design.md#the-sequence-contract]
        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr explicit sequence_adaptor(Alloc const& alloc)
        :
                m_bits(alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(size_type n, Alloc const& alloc)
        :
                m_bits(n, alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value, Alloc const& alloc)
        :
                m_bits(n, alloc)
        {
                if (value) {
                        Traits::fill(m_bits, true);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S, class Alloc>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>> and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(I first, S last, Alloc const& alloc)
        :
                m_bits(alloc)
        {
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        template<std::ranges::input_range R, class Alloc>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>> and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(std::from_range_t, R&& rg, Alloc const& alloc)
        :
                sequence_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(sequence_adaptor const& other, Alloc const& alloc)
        :
                m_bits(other.m_bits, alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(sequence_adaptor&& other, Alloc const& alloc)
        :
                m_bits(std::move(other.m_bits), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(std::initializer_list<value_type> il, Alloc const& alloc)
        :
                sequence_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires is_owner and requires (bits_type const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        constexpr auto operator=(std::initializer_list<value_type> il)
                -> sequence_adaptor&
                requires can_grow
        {
                assign(il.begin(), il.end());
                return *this;
        }

        // [sequence.reqmts]: assign in its three shapes, each a clear and a refill.
        constexpr auto assign(size_type n, value_type const& value)
                -> void
                requires can_grow
        {
                m_bits.clear();
                m_bits.resize(n, value);
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>>
        constexpr auto assign(I first, S last)
                -> void
        {
                m_bits.clear();
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        constexpr auto assign(std::initializer_list<value_type> il)
                -> void
                requires can_grow
        {
                assign(il.begin(), il.end());
        }

        // [sequence.reqmts]'s range members, and [vector]'s insert and erase, over a storage that grows. [design.md#the-range-members]
        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        constexpr auto append_range(R&& rg)
                -> void
        {
                if constexpr (blittable<std::remove_cvref_t<R>>) {
                        blit<typename std::remove_cvref_t<R>::traits_type>(rg.storage(), rg.offset(), rg.size());
                } else {
                        pack(std::forward<R>(rg));
                }
        }

        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        constexpr auto assign_range(R&& rg)
                -> void
        {
                m_bits.clear();
                append_range(std::forward<R>(rg));
        }

        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        constexpr auto insert_range(const_iterator position, R&& rg)
                -> iterator
        {
                auto const pos = index_of(position);
                return rebuild(pos, pos, [&](sequence_adaptor& tmp) -> void { tmp.append_range(std::forward<R>(rg)); });
        }

        constexpr auto insert(const_iterator position, value_type const& value)
                -> iterator
                requires can_grow
        {
                return insert(position, 1UZ, value);
        }

        constexpr auto insert(const_iterator position, size_type n, value_type const& value)
                -> iterator
                requires can_grow
        {
                auto const pos = index_of(position);
                return rebuild(pos, pos, [&](sequence_adaptor& tmp) -> void { tmp.m_bits.resize(tmp.size() + n, value); });
        }

        template<std::input_iterator I, std::sentinel_for<I> S>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>>
        constexpr auto insert(const_iterator position, I first, S last)
                -> iterator
        {
                return insert_range(position, std::ranges::subrange(first, last));
        }

        constexpr auto insert(const_iterator position, std::initializer_list<value_type> il)
                -> iterator
                requires can_grow
        {
                return insert(position, il.begin(), il.end());
        }

        constexpr auto emplace(const_iterator position, value_type const& value)
                -> iterator
                requires can_grow
        {
                return insert(position, value);
        }

        constexpr auto erase(const_iterator position)
                -> iterator
                requires can_grow
        {
                return erase(position, position + 1);
        }

        constexpr auto erase(const_iterator first, const_iterator last)
                -> iterator
                requires can_grow
        {
                return rebuild(index_of(first), index_of(last), [](sequence_adaptor&) -> void {});
        }

        [[nodiscard]] constexpr explicit sequence_adaptor(Bits& c) noexcept
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. [design.md#views-over-owners]
        template<owner_of<Bits, Traits> Owner>
        [[nodiscard]] constexpr explicit sequence_adaptor(Owner& c) noexcept
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c.m_bits)
        {}

        // [span.sub]'s three, on a view and never on an owner, since std::array and std::vector have no subviews; asserting their preconditions, dynamic_extent reaching the end. [design.md#windows]
        using subspan_type = sequence_adaptor<Bits, ownership::refers, true, Traits>;

        [[nodiscard]] constexpr auto first(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return { &storage(), offset(), count };
        }

        [[nodiscard]] constexpr auto last(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return { &storage(), offset() + (size() - count), count };
        }

        [[nodiscard]] constexpr auto subspan(size_type off, size_type count = std::dynamic_extent) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(off <= size());
                assert(count == std::dynamic_extent or count <= size() - off);
                return { &storage(), offset() + off, count == std::dynamic_extent ? size() - off : count };
        }

        // fill: the trait's entry over the whole, a masked word at a time over a window of ours, one position at a time over a window of anything else. [design.md#windows]
        constexpr auto fill(this auto&& self, value_type const& u) noexcept
                -> void
                requires (is_window and requires (std::size_t i) { Traits::unchecked_assign(self.storage(), i, u); }) or (not is_window and requires { Traits::fill(self.storage(), u); })
        {
                if constexpr (not is_window) {
                        Traits::fill(self.storage(), u);
                } else if constexpr (requires { self.storage().set(self.offset(), self.size(), u); }) {
                        self.storage().set(self.offset(), self.size(), u);
                } else {
                        for (auto i = self.offset(), last = self.offset() + self.size(); i < last; ++i) {
                                Traits::unchecked_assign(self.storage(), i, u);
                        }
                }
        }

        // The storage's own swap through the customization point, std::bitset having no member to call.
        constexpr auto swap(sequence_adaptor& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
                requires is_owner and std::swappable<Bits>
        {
                std::ranges::swap(this->m_bits, other.m_bits);
        }

        // iterators, spelled over what the accessor hands this self: deep const for an owner, shallow for a view.
        [[nodiscard]] constexpr auto begin (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), self.offset() }; }
        [[nodiscard]] constexpr auto end   (this auto&& self) noexcept -> iterator_t<decltype(self)> { return { &self.storage(), self.offset() + self.size() }; }
        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept { return std::make_reverse_iterator(self.end());   }
        [[nodiscard]] constexpr auto rend  (this auto&& self) noexcept { return std::make_reverse_iterator(self.begin()); }

        [[nodiscard]] constexpr auto cbegin()  const noexcept -> const_iterator         { return { &std::as_const(storage()), offset() }; }
        [[nodiscard]] constexpr auto cend()    const noexcept -> const_iterator         { return { &std::as_const(storage()), offset() + size() }; }
        [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cend());   }
        [[nodiscard]] constexpr auto crend()   const noexcept -> const_reverse_iterator { return std::make_reverse_iterator(cbegin()); }

        // The sequence reading a word at a time, which the range-for cannot be. [design.md#the-sequence-for-each] [design.md#the-set-for-each] [design.md#the-functor-takes-a-value]
        template<class F>
                requires std::invocable<F&, bool>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                if constexpr (block_readable<Traits, bits_type>) {
                        detail::sequence::walk_words<Traits>(self.storage(), self.offset(), self.size(), f);
                } else {
                        detail::sequence::walk_positions<Traits>(self.storage(), self.offset(), self.size(), f);
                }
        }

        // capacity; max_size() is the positions there are to hold: a growing one what its storage can address, and a static width, a view or a window their own, none of them able to grow. [design.md#max-size-is-the-bits]
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return size() == 0UZ; }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                if constexpr (is_window) {
                        return m_bits.size;
                } else {
                        return Traits::size(storage());
                }
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr (can_grow) {
                        return m_bits.max_size();
                } else {
                        return size();
                }
        }

        // The sequence reading's aggregates, in its own vocabulary and with the bool that [alg.count] and [alg.all.of] give them, where the bitset reading's four take none. [design.md#two-readings-disagree] [design.md#the-sequence-aggregates]
        [[nodiscard]] constexpr auto count(value_type value = true) const noexcept
                -> size_type
        {
                auto const n = count_true();
                return value ? n : size() - n;
        }

        [[nodiscard]] constexpr auto all (value_type value = true) const noexcept -> bool { return value ? all_true()  : none_true(); }
        [[nodiscard]] constexpr auto any (value_type value = true) const noexcept -> bool { return value ? any_true()  : not all_true(); }
        [[nodiscard]] constexpr auto none(value_type value = true) const noexcept -> bool { return value ? none_true() : all_true(); }

        // std::mismatch's answer over the machinery the orderings are already made of: the first differing block and its xor, one countr_zero from the position, and size() where the two agree, as [alg.mismatch] answers last. [design.md#the-sequence-aggregates]
        [[nodiscard]] constexpr auto mismatch(sequence_adaptor const& other) const noexcept
                -> size_type
                requires (not is_window) and requires { Traits::first_difference(std::declval<bits_type const&>(), std::declval<bits_type const&>()); }
        {
                assert(size() == other.size());
                // A zero width has no blocks to ask about, and answers its own width, which is nought.
                if (empty()) {
                        return 0UZ;
                }
                auto const [ index, diff ] = Traits::first_difference(storage(), other.storage());
                using block_type = std::remove_cvref_t<decltype(diff)>;
                constexpr auto digits = static_cast<size_type>(std::numeric_limits<block_type>::digits);
                if (diff == block_type{}) {
                        return size();
                }
                return (index * digits) + detail::bits::countr_zero(diff);
        }

        // Growth, [vector]'s members over storage that spells them alike, so detected on the storage rather than reconciled by the trait. [design.md#growth]
        constexpr auto resize(size_type n)                          -> void requires can_grow { m_bits.resize(n); }
        constexpr auto resize(size_type n, value_type const& value) -> void requires can_grow { m_bits.resize(n, value); }
        constexpr auto clear() noexcept                             -> void requires can_grow { m_bits.clear(); }
        constexpr auto push_back(value_type const& value)           -> void requires can_grow { m_bits.push_back(value); }
        constexpr auto pop_back() noexcept                          -> void requires can_grow { m_bits.pop_back(); }

        constexpr auto emplace_back(value_type const& value)
                -> reference
                requires can_grow
        {
                m_bits.push_back(value);
                return back();
        }

        constexpr auto reserve(size_type n)
                -> void
                requires can_grow and requires (bits_type& b) { b.reserve(n); }
        {
                m_bits.reserve(n);
        }

        [[nodiscard]] constexpr auto capacity() const noexcept
                -> size_type
                requires can_grow and requires (bits_type const& b) { b.capacity(); }
        {
                return m_bits.capacity();
        }

        constexpr auto shrink_to_fit()
                -> void
                requires can_grow and requires (bits_type& b) { b.shrink_to_fit(); }
        {
                m_bits.shrink_to_fit();
        }

        // element access, [] unchecked and at() throwing as [array] has them. [design.md#asking-is-total]
        [[nodiscard]] constexpr auto operator[](this auto&& self, size_type n) noexcept
                -> reference_t<decltype(self)>
        {
                assert(n < self.size());
                return { &self.storage(), self.offset() + n };
        }

        [[nodiscard]] constexpr auto at(this auto&& self, size_type n)
                -> reference_t<decltype(self)>
        {
                if (n < self.size()) {
                        return { &self.storage(), self.offset() + n };
                }
                throw out_of_range(n, self.size());
        }

        [[nodiscard]] constexpr auto front(this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), self.offset() }; }
        [[nodiscard]] constexpr auto back (this auto&& self) noexcept -> reference_t<decltype(self)> { return { &self.storage(), self.offset() + self.size() - 1UZ }; }

        // The owner's alone, following span: a handle declines to say whether it compares its referent or its contents. Defaulted, the storage being the one member. [design.md#views-follow-their-precedent]
        [[nodiscard]] friend constexpr auto operator==(sequence_adaptor const& x, sequence_adaptor const& y) noexcept -> bool requires is_owner = default;

        // The trait's entry and nothing else: an owner is over storage of ours, which has one. [design.md#owning-is-ours]
        [[nodiscard]] friend constexpr auto operator<=>(sequence_adaptor const& x, sequence_adaptor const& y) noexcept
                -> std::strong_ordering
                requires is_owner and requires { Traits::sequence_three_way(x.storage(), y.storage()); }
        {
                return Traits::sequence_three_way(x.storage(), y.storage());
        }

        // Bulk, on the storage's own spelling: on packed bits the pointwise operation and the set operation are one instruction; not on a window, whose blocks are not its own. [design.md#what-the-trait-reconciles]
        constexpr auto operator&=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() &= other.storage(); } { self.storage() &= other.storage(); return self; }
        constexpr auto operator|=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() |= other.storage(); } { self.storage() |= other.storage(); return self; }
        constexpr auto operator^=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() ^= other.storage(); } { self.storage() ^= other.storage(); return self; }
        constexpr auto operator-=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() -= other.storage(); } { self.storage() -= other.storage(); return self; }

        constexpr auto operator<<=(this auto&& self, std::size_t n) noexcept -> auto& requires (not is_window) and requires { self.storage() <<= n; } { self.storage() <<= n; return self; }
        constexpr auto operator>>=(this auto&& self, std::size_t n) noexcept -> auto& requires (not is_window) and requires { self.storage() >>= n; } { self.storage() >>= n; return self; }

        // Bulk on a window of ours, against a source of any shape read by block: a word at a time at either alignment, through word_at and set_word; equal sizes, and no overlap short of coincidence. [design.md#windows]
        template<class Other> constexpr auto operator&=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and word_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a & b);  }); return self; }
        template<class Other> constexpr auto operator|=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and word_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a | b);  }); return self; }
        template<class Other> constexpr auto operator^=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and word_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a ^ b);  }); return self; }
        template<class Other> constexpr auto operator-=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and word_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a & static_cast<decltype(b)>(~b)); }); return self; }

        // [vector.bool]'s two: flip every bit, a bulk operation like the ones above, and swap two proxies, which the proxies' own swap already does.
        constexpr auto flip(this auto&& self) noexcept -> void requires (not is_window) and requires { self.storage().flip(); } { self.storage().flip(); }

        static constexpr auto swap(reference x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }

private:
        // One tier each for the three aggregates above, chosen once: the trait's door over the whole, a masked word at a time over a window of ours, one position at a time over a window of anything else. [design.md#windows]
        [[nodiscard]] constexpr auto count_true() const noexcept
                -> size_type
        {
                if constexpr (not is_window) {
                        return detail::bits::count<Traits>(storage());
                } else if constexpr (block_readable<Traits, bits_type>) {
                        return detail::sequence::count_words<Traits>(storage(), offset(), size());
                } else {
                        return detail::sequence::count_positions<Traits>(storage(), offset(), size());
                }
        }

        [[nodiscard]] constexpr auto any_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return detail::bits::any<Traits>(storage());
                } else if constexpr (block_readable<Traits, bits_type>) {
                        return detail::sequence::any_words<Traits>(storage(), offset(), size());
                } else {
                        return detail::sequence::any_positions<Traits>(storage(), offset(), size(), true);
                }
        }

        // Its own helper rather than not any_true(), so a storage that spells none() itself is asked in its own words; every storage adapted here does.
        [[nodiscard]] constexpr auto none_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return detail::bits::none<Traits>(storage());
                } else if constexpr (block_readable<Traits, bits_type>) {
                        return not detail::sequence::any_words<Traits>(storage(), offset(), size());
                } else {
                        return not detail::sequence::any_positions<Traits>(storage(), offset(), size(), true);
                }
        }

        // Not count() == size(): a clear position ends it, which is what the position tier looks for and what a word that is not all ones is.
        [[nodiscard]] constexpr auto all_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return detail::bits::all<Traits>(storage());
                } else if constexpr (block_readable<Traits, bits_type>) {
                        return detail::sequence::all_words<Traits>(storage(), offset(), size());
                } else {
                        return not detail::sequence::any_positions<Traits>(storage(), offset(), size(), false);
                }
        }

        // The words of this window against the words of another at its own alignment, each masked to what the window holds.
        template<class Other, class F>
        constexpr auto combine(this auto&& self, Other const& other, F f) noexcept
                -> void
        {
                using block_type = bits_type::block_type;
                constexpr auto digits = bits_type::bits_per_block;
                assert(self.size() == other.size());
                for (auto k = 0UZ; k < self.size(); k += digits) {
                        auto const mask = detail::sequence::word_mask<block_type>(std::ranges::min(digits, self.size() - k));
                        auto const mine   = detail::bits::word_at<Traits>(self.storage(), self.offset() + k);
                        auto const theirs = detail::bits::word_at<typename Other::traits_type>(other.storage(), other.offset() + k);
                        self.storage().set_word(self.offset() + k, f(mine, theirs), mask);
                }
        }

        // Tier one: the source's bits as words at its own alignment, appended a word at a time and trimmed to the count; a source in this very storage reads only below the old width, which no append touches. [design.md#the-blit]
        template<class STraits, class SBits>
        constexpr auto blit(SBits const& src, size_type first, size_type count)
                -> void
        {
                constexpr auto digits = bits_type::bits_per_block;
                auto const old = size();
                if constexpr (requires (bits_type& b, std::size_t n) { b.reserve(n); }) {
                        m_bits.reserve(old + count);
                }
                for (auto pos = first; pos < first + count; pos += digits) {
                        m_bits.append(detail::bits::word_at<STraits>(src, pos));
                }
                m_bits.resize(old + count);
        }

        // Tier two: the bools packed into words, boost's bit_appender, and the last word trimmed to what it holds.
        template<std::ranges::input_range R>
        constexpr auto pack(R&& rg)
                -> void
        {
                using block_type = bits_type::block_type;
                constexpr auto digits = bits_type::bits_per_block;
                if constexpr (std::ranges::sized_range<R> and requires (bits_type& b, std::size_t n) { b.reserve(n); }) {
                        m_bits.reserve(size() + std::ranges::size(rg));
                }
                auto word = block_type{};
                auto n = 0UZ;
                for (auto&& e : rg) {
                        if (static_cast<value_type>(e)) {
                                word |= detail::bits::shl(block_type{1}, n);
                        }
                        if (++n == digits) {
                                m_bits.append(word);
                                word = block_type{};
                                n = 0UZ;
                        }
                }
                if (n != 0UZ) {
                        auto const total = size() + n;
                        m_bits.append(word);
                        m_bits.resize(total);
                }
        }

        // Rebuilt rather than shifted: head, the middle the caller appends, tail, then one swap, so the strong guarantee comes free. [design.md#the-range-members]
        template<class Middle>
        constexpr auto rebuild(size_type pos, size_type tail, Middle&& middle)
                -> iterator
        {
                auto const whole = sequence_adaptor<bits_type const, ownership::refers, false, Traits>(std::as_const(storage()));
                auto tmp = sequence_adaptor();
                tmp.append_range(whole.first(pos));
                middle(tmp);
                tmp.append_range(whole.subspan(tail));
                swap(tmp);
                return begin() + static_cast<difference_type>(pos);
        }

        [[nodiscard]] constexpr auto index_of(const_iterator position) const noexcept
                -> size_type
        {
                return static_cast<size_type>(position - cbegin());
        }

        static constexpr auto out_of_range(std::size_t n, std::size_t size, std::source_location const& loc = std::source_location::current())
        {
                return std::out_of_range(
                        std::format(
                                "{}:{}:{}: exception: ‘{}‘: argument ‘n‘ is out of range [{} >= {}]",
                                loc.file_name(), loc.line(), loc.column(), loc.function_name(), n, size
                        )
                );
        }
};

// A view deduces the constness of what it views, the way span<T> and span<T const> do; over an owner, of the storage it wraps. [design.md#a-bitset-reads-as-its-storage]
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
sequence_adaptor(Bits&) -> sequence_adaptor<Bits, ownership::refers, false>;

template<class Owner>
        requires requires { typename owned_storage<std::remove_const_t<Owner>>::bits_type; }
sequence_adaptor(Owner&) -> sequence_adaptor<owned_bits_t<Owner>, ownership::refers, false, owned_traits_t<Owner>>;

// The owner's side of the protocol above.
template<class Bits, class Traits>
struct owned_storage<sequence_adaptor<Bits, ownership::owns, false, Traits>>
{
        using bits_type   = Bits;
        using traits_type = Traits;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports as redundant.
template<class Bits, ownership Own, bool Windowed, class Traits>
constexpr auto swap(sequence_adaptor<Bits, Own, Windowed, Traits>& x, sequence_adaptor<Bits, Own, Windowed, Traits>& y) noexcept(noexcept(x.swap(y)))
        -> void
        requires (owns(Own))
{
        x.swap(y);
}
// NOLINTEND(readability-redundant-parentheses)

// [vector.erasure], over the owner's own erase: the proxies move and swap, so remove_if runs unchanged over the packed bits. [design.md#the-sequence-contract]
template<class Bits, ownership Own, bool Windowed, class Traits, class Pred>
constexpr auto erase_if(sequence_adaptor<Bits, Own, Windowed, Traits>& c, Pred pred)
        -> sequence_adaptor<Bits, Own, Windowed, Traits>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        auto const [first, last] = std::ranges::remove_if(c, pred);
        auto const n = static_cast<sequence_adaptor<Bits, Own, Windowed, Traits>::size_type>(last - first);
        c.erase(first, last);
        return n;
}

template<class Bits, ownership Own, bool Windowed, class Traits, class U = bool>
constexpr auto erase(sequence_adaptor<Bits, Own, Windowed, Traits>& c, U const& value)
        -> sequence_adaptor<Bits, Own, Windowed, Traits>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        return xstd::erase_if(c, [&](bool x) -> bool { return x == value; });
}

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright and borrowed, as set_adaptor's is. [design.md#views-follow-their-precedent]
template<class Bits, bool Windowed, class Traits>
inline constexpr bool enable_view<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed, Traits>> = true;

template<class Bits, bool Windowed, class Traits>
inline constexpr bool enable_borrowed_range<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed, Traits>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// The owner hashes as std::vector<bool> does; a view no more than std::span does. [design.md#the-hashing-invariant]
template<class Bits, bool Windowed, class Traits>
struct hash<xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed, Traits>>
{
        [[nodiscard]] constexpr auto operator()(xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed, Traits> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

}       // namespace std
// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload, which cannot hash the proxy the iterator returns. [design.md#the-hashing-invariant]
namespace boost::container_hash {

template<class Bits, xstd::ownership Own, bool Windowed, class Traits>
struct is_range<xstd::sequence_adaptor<Bits, Own, Windowed, Traits>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_SEQUENCE_ADAPTOR_HPP
