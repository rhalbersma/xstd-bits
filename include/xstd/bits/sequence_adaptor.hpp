//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SEQUENCE_ADAPTOR_HPP
#define XSTD_BITS_SEQUENCE_ADAPTOR_HPP

#include <xstd/bits/detail/allocator_base_type.hpp> // allocator_base_type
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/hash.hpp>              // hash_append_bits, std_hash
#include <xstd/bits/detail/intrin.hpp>            // countr_zero, popcount
#include <xstd/bits/detail/shift.hpp>             // shl, shr
#include <xstd/bits/detail/random_access.hpp>     // random_access_bit_iterator, random_access_bit_reference
#include <xstd/bits/ownership.hpp>                // owned_bits_t, owned_storage, owner_of, owner_reading, ownership, owns, reading
#include <xstd/misc/concepts/specialization_of.hpp> // specialization_of_TN
#include <xstd/misc/type_traits/empty_base_type.hpp>          // empty_base_type
#include <boost/container_hash/is_range.hpp>      // is_range
#include <boost/hash2/hash_append.hpp>            // hash_append_tag
#include <algorithm>                              // copy, min, remove_if
#include <cassert>                                // assert
#include <compare>                                // strong_ordering
#include <concepts>                               // constructible_from, invocable, same_as, swap, swappable
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

// The sequence reading, [array] over a contiguous_bit_container, owning it or referring to it.
namespace xstd {

namespace detail::sequence {

// The bits a window's word holds: every one but for the last word, which holds what is left over.
template<class Block>
[[nodiscard]] constexpr auto partial_block_mask(std::size_t count) noexcept
        -> Block
{
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<Block>::digits);
        // No shift by digits, which is undefined: a full word is every bit, spelled without one.
        return count == digits ? static_cast<Block>(~Block{}) : static_cast<Block>(detail::bits::shl(Block{1}, count) - Block{1});
}

// Continue unless the functor says otherwise: a void functor always continues, a bool one says.
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

// Every position, lowest first, a word at a time: the outer loop loads once per word and the reload becomes the inner loop's exit test, which a flat operator++ can never express. There is no second tier below any more -- these walked words where the storage read by block and positions where it did not, and only the first kind of storage can be here.
template<class Bits, class F>
constexpr auto walk_blocks(Bits const& c, std::size_t offset, std::size_t size, F& f)
        -> void
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const count = std::ranges::min(digits, size - k);
                auto const block = c.block_at(offset + k);
                for (auto n = 0UZ; n < count; ++n) {
                        if (not invoke_continues(f, (detail::bits::shr(block, n) & block_type{1}) != block_type{})) {
                                return;
                        }
                }
        }
}

// The three aggregates over a window, each masked to what the window holds: a word at a time, so a window pays what the whole pays and not a test per bit.
template<class Bits>
[[nodiscard]] constexpr auto count_blocks(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> std::size_t
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        auto n = 0UZ;
        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = partial_block_mask<block_type>(std::ranges::min(digits, size - k));
                n += detail::bits::popcount(static_cast<block_type>(c.block_at(offset + k) & mask));
        }
        return n;
}

template<class Bits>
[[nodiscard]] constexpr auto any_blocks(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> bool
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = partial_block_mask<block_type>(std::ranges::min(digits, size - k));
                if (static_cast<block_type>(c.block_at(offset + k) & mask) != block_type{}) {
                        return true;
                }
        }
        return false;
}

template<class Bits>
[[nodiscard]] constexpr auto all_blocks(Bits const& c, std::size_t offset, std::size_t size) noexcept
        -> bool
{
        using block_type = std::remove_cvref_t<decltype(c.block(0UZ))>;
        constexpr auto digits = static_cast<std::size_t>(std::numeric_limits<block_type>::digits);

        for (auto k = 0UZ; k < size; k += digits) {
                auto const mask = partial_block_mask<block_type>(std::ranges::min(digits, size - k));
                if (static_cast<block_type>(c.block_at(offset + k) & mask) != mask) {
                        return false;
                }
        }
        return true;
}

// The storage's block type, spelled where no typename is needed: P0634 made an alias-declaration a context in which only a type can appear, and a template argument is not one -- GCC rejects the same elision there, so the alias is what lets the specialization below read as it does.
template<class Bits>
using block_type_of = std::remove_const_t<Bits>::block_type;

}       // namespace detail::sequence

// An owner names its storage's allocator, as std::vector<bool> names its own; a view names none, owning nothing.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, ownership Own, bool Windowed>
class sequence_adaptor;

// A sequence adaptor of any shape whose storage holds blocks of the given type: what a blit reads, and nothing else, since only an adaptor hands its storage to another.
template<class S, class Block>
inline constexpr bool blit_source = false;

template<class Bits, ownership Own, bool Windowed, class Block>
inline constexpr bool blit_source<sequence_adaptor<Bits, Own, Windowed>, Block> = std::same_as<detail::sequence::block_type_of<Bits>, Block>;

template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, ownership Own, bool Windowed>
class sequence_adaptor : public std::conditional_t<owns(Own), detail::bits::allocator_base_type<std::remove_const_t<Bits>>, xstd::empty_base_type<>>
{
        static constexpr bool is_owner  = owns(Own);
        static constexpr bool is_window = Windowed;
        static_assert(not (is_owner and is_window), "a window views what another owns");

        using bits_type = std::remove_const_t<Bits>;

        // A width fixed at compile time, which is what the byte exchange below asks and what can_grow is the absence of.
        static constexpr bool has_static_width = (bits_type::extent != std::dynamic_extent);

        // Growth is the owner's over storage that grows: a view must never resize what it does not own.
        static constexpr bool can_grow = is_owner and not has_static_width and requires (bits_type& b, std::size_t n, bool value) { b.resize(n, value); b.push_back(value); b.pop_back(); b.clear(); };

        // A window is what std::span stores, the pointer's role split over a pointer and a position because bits are not addressable: the iterator's two fields and a size.
        struct window
        {
                Bits* ptr;
                std::size_t offset;
                std::size_t size;
        };

        std::conditional_t<is_owner, Bits, std::conditional_t<is_window, window, Bits*>> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow, a window's pointer likewise.
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

        // Where this sequence starts in the storage: zero but for a window, so every position below is offset once, here.
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

        template<class Self> using iterator_t  = detail::bits::random_access_bit_iterator <storage_t<Self>>;
        template<class Self> using reference_t = detail::bits::random_access_bit_reference<storage_t<Self>>;

        // A source the blit can read by block, of this storage's own block type.
        template<class S>
        static constexpr bool blittable = blit_source<S, typename bits_type::block_type>;

        // A storage that takes a masked word at any position: ours, which is what a window's bulk operators write through.
        // Asked of Bits and not bits_type, which has the const stripped off it: a window over a const storage holds it by a pointer to const, so the masked write is what it cannot do and what this must answer no to.
        static constexpr bool block_writable = requires (Bits& b, std::size_t pos, bits_type::block_type w) { b.block_at(pos, w, w); };

        // A sequence view refers into this owner's storage, and nothing else outside does; a set view does not, the readings not mixing.
        template<specialization_of_TN<detail::bits::contiguous_bit_container> B, ownership O, bool W> friend class sequence_adaptor;

        // The value under the sequence reading, the owner's alone as == is: a view follows span and hashes no more than it compares.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, sequence_adaptor const* v) noexcept
                -> void
                requires is_owner
        {
                detail::bits::hash_append_bits(h, f, v->storage());
        }

public:
        // types
        using value_type             = bool;
        using pointer                = void;
        using const_pointer          = pointer;
        using reference              = detail::bits::random_access_bit_reference<Bits>;
        using const_reference        = detail::bits::random_access_bit_reference<Bits const>;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using iterator               = detail::bits::random_access_bit_iterator<Bits>;
        using const_iterator         = detail::bits::random_access_bit_iterator<Bits const>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy; an owner is built the way std::array is, or std::vector where the storage grows, a view only from what it views.
        [[nodiscard]] constexpr sequence_adaptor() noexcept requires is_owner = default;

        [[nodiscard]] constexpr explicit sequence_adaptor(size_type n)
                requires can_grow
        :
                m_bits(bits_type::check_addressable_width(n))
        {}

        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value)
                requires can_grow
        :
                m_bits(bits_type::check_addressable_width(n))
        {
                if (value) {
                        m_bits.fill(true);
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

        // std::array's aggregate initialization, as a constructor: what is listed leads and the rest stays false, a longer list being the error it is there.
        constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires is_owner and (not can_grow)
        :
                m_bits()
        {
                assert(il.size() <= size());
                std::ranges::copy(il, begin());
        }

        // A field of bits in, a field of bits out, in the currency the three readings share: byte j holds the
        // positions [8j, 8j + 8) least significant bit first, so a fixed width over the same positions agrees byte
        // for byte with any other and this is a copy rather than a walk. EXPLICIT in both directions, as it is under
        // the set reading -- a packed array of bool and a field of bits are two readings of the same bits, and this
        // library makes a reader pick one rather than letting a conversion pick for them.
        //
        // NOT ON A WINDOW, and that is the whole of why is_window is asked. A window is a bit offset and a size of
        // its own into storage it does not span: its position zero is not the storage's, so its bytes are not the
        // storage's bytes and to_bits would hand back the wrong ones. The width test inside exchanges_bits does not
        // catch it, since a window over a static container reports the CONTAINER's extent rather than its own size. A view
        // that is not a window spans the whole container, so its bytes are that container's and it converts.
        template<class B>
                requires is_owner and bits_type::template exchanges_bits<B>
        [[nodiscard]] constexpr explicit sequence_adaptor(B const& b) noexcept
        {
                storage().assign_bits(b);
        }

        template<class B>
                requires (not is_window) and bits_type::template exchanges_bits<B>
        [[nodiscard]] constexpr explicit operator B() const noexcept
        {
                return storage().template to_bits<B>();
        }

        // [vector.bool]'s allocator arguments, where the storage takes one: deduced and matched, so a storage without one has no such constructor.
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
                m_bits(bits_type::check_addressable_width(n), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value, Alloc const& alloc)
        :
                m_bits(bits_type::check_addressable_width(n), alloc)
        {
                if (value) {
                        m_bits.fill(true);
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
                m_bits.resize(bits_type::check_addressable_width(n), value);
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

        // [sequence.reqmts]'s range members, and [vector]'s insert and erase, over a storage that grows.
        template<std::ranges::input_range R>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>>
        constexpr auto append_range(R&& rg)
                -> void
        {
                if constexpr (blittable<std::remove_cvref_t<R>>) {
                        blit(rg.storage(), rg.offset(), rg.size());
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
                // Through the storage's saturating sum: n is a count the caller names, so tmp.size() + n wraps, and a wrapped total would resize the copy down and answer an insertion with a shorter sequence than it started from.
                return rebuild(pos, pos, [&](sequence_adaptor& tmp) -> void { tmp.m_bits.resize(bits_type::check_addressable_width(bits_type::width_sum(tmp.size(), n)), value); });
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
                // Dereferenceable, which cend() is not: [sequence.reqmts] asks that of the single-position erase, and position + 1 past it is a range index_of would have to answer for.
                assert(position != cend());
                return erase(position, position + 1);
        }

        constexpr auto erase(const_iterator first, const_iterator last)
                -> iterator
                requires can_grow
        {
                // A range, not two positions: index_of establishes that each is one of ours, and this that they are in that order, which the rebuild's tail subtraction needs and neither of them says.
                assert(first <= last);
                return rebuild(index_of(first), index_of(last), [](sequence_adaptor&) -> void {});
        }

        [[nodiscard]] constexpr explicit sequence_adaptor(Bits& c) noexcept
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps, the owner having befriended this template. Implicit, unlike the one above: it asserts nothing the owner does not already carry, which is the line span draws.
        template<owner_of<Bits, reading::sequence> Owner>
        [[nodiscard]] constexpr explicit(false) sequence_adaptor(Owner& c) noexcept  // NOLINT(misc-explicit-constructor)
                requires (not is_owner) and (not is_window)
        :
                m_bits(&c.m_bits)
        {}

        // [span.sub]'s three, on a view and never on an owner, since std::array and std::vector have no subviews; asserting their preconditions, dynamic_extent reaching the end.
        using subspan_type = sequence_adaptor<Bits, ownership::refers, true>;

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

        // fill: the trait's entry over the whole, a masked word at a time over a window of ours, one position at a time over a window of anything else.
        constexpr auto fill(this auto&& self, value_type const& u) noexcept
                -> void
                requires (is_window and requires (std::size_t i) { self.storage().assign(i, u); }) or (not is_window and requires { self.storage().fill(u); })
        {
                if constexpr (not is_window) {
                        self.storage().fill(u);
                } else if constexpr (requires { self.storage().set(self.offset(), self.size(), u); }) {
                        self.storage().set(self.offset(), self.size(), u);
                } else {
                        for (auto i = self.offset(), last = self.offset() + self.size(); i < last; ++i) {
                                self.storage().assign(i, u);
                        }
                }
        }

        // The non-member beside it, hidden though the operators here are namespace-scope templates: ranges::swap finds this and never the member, and xstd::swap(a, b) is a spelling people reach for by habit where a qualified operator is not.
        friend constexpr auto swap(sequence_adaptor& x, sequence_adaptor& y) noexcept(noexcept(x.swap(y)))
                -> void
                requires is_owner
        {
                x.swap(y);
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

        // The sequence reading a word at a time, which the range-for cannot be.
        template<class F>
                requires std::invocable<F&, bool>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                detail::sequence::walk_blocks(self.storage(), self.offset(), self.size(), f);
        }

        // capacity; max_size() is the positions there are to hold: a growing one what its storage can address, and a static width, a view or a window their own, none of them able to grow.
        [[nodiscard]] constexpr auto empty() const noexcept -> bool { return size() == 0UZ; }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                if constexpr (is_window) {
                        return m_bits.size;
                } else {
                        return storage().size();
                }
        }

        // std::vector<bool>'s answer where this reading can grow, and the width itself where it cannot: the storage computes both ceilings and this reading picks the one its counterpart names, a random access range's positions being counted by a difference_type. A view is its own ceiling, growing nothing.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
        {
                if constexpr (can_grow) {
                        return m_bits.addressable_max_size();
                } else {
                        return size();
                }
        }

        // The sequence reading's aggregates, in its own vocabulary and with the bool that [alg.count] and [alg.all.of] give them, where the bitset reading's four take none.
        [[nodiscard]] constexpr auto count(value_type value = true) const noexcept
                -> size_type
        {
                auto const n = count_true();
                return value ? n : size() - n;
        }

        [[nodiscard]] constexpr auto all (value_type value = true) const noexcept -> bool { return value ? all_true()  : none_true(); }
        [[nodiscard]] constexpr auto any (value_type value = true) const noexcept -> bool { return value ? any_true()  : not all_true(); }
        [[nodiscard]] constexpr auto none(value_type value = true) const noexcept -> bool { return value ? none_true() : all_true(); }

        // std::mismatch's answer over the machinery the orderings are already made of: the first differing block and its xor, one countr_zero from the position, and size() where the two agree, as [alg.mismatch] answers last.
        [[nodiscard]] constexpr auto mismatch(sequence_adaptor const& other) const noexcept
                -> size_type
                requires (not is_window) and requires (bits_type const& b) { b.first_difference(b); }
        {
                assert(size() == other.size());
                // A zero width has no blocks to ask about, and answers its own width, which is nought.
                if (empty()) {
                        return 0UZ;
                }
                auto const [ index, diff ] = storage().first_difference(other.storage());
                using block_type = std::remove_cvref_t<decltype(diff)>;
                constexpr auto digits = static_cast<size_type>(std::numeric_limits<block_type>::digits);
                if (diff == block_type{}) {
                        return size();
                }
                return (index * digits) + detail::bits::countr_zero(diff);
        }

        // Growth, [vector]'s members over storage that spells them alike, so detected on the storage rather than reconciled by the trait. The storage computes the ceiling and this reading is what asks it, because the choice of ceiling is this reading's: std::vector<bool> throws length_error for a size it cannot represent, and the bitset reading beside it asks none and answers bad_alloc as boost does.
        constexpr auto resize(size_type n)                          -> void requires can_grow { m_bits.resize(bits_type::check_addressable_width(n)); }
        constexpr auto resize(size_type n, value_type const& value) -> void requires can_grow { m_bits.resize(bits_type::check_addressable_width(n), value); }
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
                m_bits.reserve(bits_type::check_addressable_width(n));
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

        // element access, [] unchecked and at() throwing as [array] has them.
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

        // Both are preconditions in [sequence.reqmts], and back()'s is the one that subtracts: on an empty sequence offset() + size() - 1UZ wraps, and the reference handed back names a position no storage has. Said at the member the caller named rather than left to the storage's own assert a call down, for the reason operator[] says n < size() where test(n) would say it again. Spelled over four lines apiece, rather than the one each was, because the coverage gate excludes an assert by a pattern anchored at the start of a line.
        [[nodiscard]] constexpr auto front(this auto&& self) noexcept
                -> reference_t<decltype(self)>
        {
                assert(not self.empty());
                return { &self.storage(), self.offset() };
        }

        [[nodiscard]] constexpr auto back(this auto&& self) noexcept
                -> reference_t<decltype(self)>
        {
                assert(not self.empty());
                return { &self.storage(), self.offset() + self.size() - 1UZ };
        }

        // The owner's alone, following span: a handle declines to say whether it compares its referent or its contents. Defaulted, the storage being the one member.
        [[nodiscard]] friend constexpr auto operator==(sequence_adaptor const& x, sequence_adaptor const& y) noexcept -> bool requires is_owner = default;

        // The storage's entry and nothing else: an owner is over storage of ours, which has one. Spelled over bits_type rather than over x.storage(), which MSVC completes eagerly here and so cannot.
        [[nodiscard]] friend constexpr auto operator<=>(sequence_adaptor const& x, sequence_adaptor const& y) noexcept
                -> std::strong_ordering
                requires is_owner and requires (bits_type const& b) { sequence_lexicographical_compare_three_way(b, b); }
        {
                return sequence_lexicographical_compare_three_way(x.storage(), y.storage());
        }

        // Elementwise logical, which is what a bitwise operator on a sequence of bools means and what std::valarray<bool> is the standard's one model for; on packed bits it is the set operation's instruction, so the reading costs nothing to serve. Three, not four: a difference is set vocabulary and has no elementwise reading, valarray's own operator-= being arithmetic.
        constexpr auto operator&=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() &= other.storage(); } { self.storage() &= other.storage(); return self; }
        constexpr auto operator|=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() |= other.storage(); } { self.storage() |= other.storage(); return self; }
        constexpr auto operator^=(this auto&& self, sequence_adaptor const& other) noexcept -> auto& requires (not is_window) and requires { self.storage() ^= other.storage(); } { self.storage() ^= other.storage(); return self; }

        // No shifts, at any shape: a shift is the bitset reading's truncating bit string and the set reading's translation, and the sequence reading already spells moving elements std::shift_left and std::shift_right -- in the opposite direction from the operators.

        // Bulk on a window of ours, against a source of any shape read by block: a word at a time at either alignment, through block_at and block_at; equal sizes, and no overlap short of coincidence.
        template<class Other> constexpr auto operator&=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and block_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a & b);  }); return self; }
        template<class Other> constexpr auto operator|=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and block_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a | b);  }); return self; }
        template<class Other> constexpr auto operator^=(this auto&& self, Other const& other) noexcept -> auto& requires is_window and block_writable and blittable<Other> { self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a ^ b);  }); return self; }

        // [vector.bool]'s two: flip every bit, a bulk operation like the ones above, and swap two proxies, which the proxies' own swap already does.
        constexpr auto flip(this auto&& self) noexcept -> void requires (not is_window) and requires { self.storage().flip(); } { self.storage().flip(); }

        static constexpr auto swap(reference x, reference y) noexcept -> void { bool const t = x; x = y; y = t; }

private:
        // One tier each for the three aggregates above, chosen once: the trait's door over the whole, a masked word at a time over a window of ours, one position at a time over a window of anything else.
        [[nodiscard]] constexpr auto count_true() const noexcept
                -> size_type
        {
                if constexpr (not is_window) {
                        return storage().count();
                } else {
                        return detail::sequence::count_blocks(storage(), offset(), size());
                }
        }

        [[nodiscard]] constexpr auto any_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return storage().any();
                } else {
                        return detail::sequence::any_blocks(storage(), offset(), size());
                }
        }

        // Its own helper rather than not any_true(), so a storage that spells none() itself is asked in its own words; every storage adapted here does.
        [[nodiscard]] constexpr auto none_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return storage().none();
                } else {
                        return not detail::sequence::any_blocks(storage(), offset(), size());
                }
        }

        // Not count() == size(): a clear position ends it, which is what a word that is not all ones says in one test.
        [[nodiscard]] constexpr auto all_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return storage().all();
                } else {
                        return detail::sequence::all_blocks(storage(), offset(), size());
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
                        auto const mask = detail::sequence::partial_block_mask<block_type>(std::ranges::min(digits, self.size() - k));
                        auto const mine   = self.storage().block_at(self.offset() + k);
                        auto const theirs = other.storage().block_at(other.offset() + k);
                        self.storage().block_at(self.offset() + k, f(mine, theirs), mask);
                }
        }

        // Tier one: the source's bits as words at its own alignment, appended a word at a time and trimmed to the count; a source in this very storage reads only below the old width, which no append touches.
        template<class SBits>
        constexpr auto blit(SBits const& src, size_type first, size_type count)
                -> void
        {
                constexpr auto digits = bits_type::bits_per_block;
                auto const old = size();
                // Through the storage's saturating sum, as every width this reading computes is: count is the source's own, so a wrapped total would resize this sequence DOWN and answer an append with something shorter than it started from.
                auto const total = bits_type::check_addressable_width(bits_type::width_sum(old, count));
                if constexpr (requires (bits_type& b, std::size_t n) { b.reserve(n); }) {
                        m_bits.reserve(total);
                }
                for (auto pos = first; pos < first + count; pos += digits) {
                        m_bits.append(src.block_at(pos));
                }
                m_bits.resize(total);
        }

        // Tier two: the bools packed into words, boost's bit_appender, and the last word trimmed to what it holds.
        template<std::ranges::input_range R>
        constexpr auto pack(R&& rg)
                -> void
        {
                using block_type = bits_type::block_type;
                constexpr auto digits = bits_type::bits_per_block;
                // Saturating for the reason blit's total is, and here the width is the caller's own to name: a sized range says how many it has without holding them, so size() + that is the one sum in this reading a caller can wrap on purpose. Wrapped it under-reserves to nothing and the appends below then run out the range one word at a time; saturated it is the length_error reserve already throws.
                if constexpr (std::ranges::sized_range<R> and requires (bits_type& b, std::size_t n) { b.reserve(n); }) {
                        m_bits.reserve(bits_type::check_addressable_width(bits_type::width_sum(size(), static_cast<std::size_t>(std::ranges::size(rg)))));
                }
                auto block = block_type{};
                auto n = 0UZ;
                for (auto&& e : rg) {
                        if (static_cast<value_type>(e)) {
                                block |= detail::bits::shl(block_type{1}, n);
                        }
                        if (++n == digits) {
                                m_bits.append(block);
                                block = block_type{};
                                n = 0UZ;
                        }
                }
                if (n != 0UZ) {
                        auto const total = size() + n;
                        m_bits.append(block);
                        m_bits.resize(total);
                }
        }

        // Rebuilt rather than shifted: head, the middle the caller appends, tail, then one swap, so the strong guarantee comes free.
        template<class Middle>
        constexpr auto rebuild(size_type pos, size_type tail, Middle&& middle)
                -> iterator
        {
                auto const whole = sequence_adaptor<bits_type const, ownership::refers, false>(std::as_const(storage()));
                auto tmp = sequence_adaptor();
                tmp.append_range(whole.first(pos));
                middle(tmp);
                tmp.append_range(whole.subspan(tail));
                swap(tmp);
                return begin() + static_cast<difference_type>(pos);
        }

        // The one place a caller's iterator becomes an index, so [sequence.reqmts]'s precondition on it is said once here rather than at each of the five members that pass one. An iterator into another sequence is already the iterator's own assert, m_ptr against m_ptr; what is left is this one, and past either end the subtraction below is a size_type that wraps or an index the rebuild then writes through.
        [[nodiscard]] constexpr auto index_of(const_iterator position) const noexcept
                -> size_type
        {
                assert(cbegin() <= position and position <= cend());
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

// A view deduces the constness of what it views, the way span<T> and span<T const> do; over an owner, of the storage it wraps.
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
sequence_adaptor(Bits&) -> sequence_adaptor<Bits, ownership::refers, false>;

template<owner_reading<reading::sequence> Owner>
sequence_adaptor(Owner&) -> sequence_adaptor<owned_bits_t<Owner>, ownership::refers, false>;

// The owner's side of the protocol above.
template<class Bits>
struct owned_storage<sequence_adaptor<Bits, ownership::owns, false>>
{
        using bits_type   = Bits;

        // Committed to the sequence reading, so only a sequence view refers into one.
        static constexpr auto reads = reading::sequence;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the requires-clause needs the parentheses the check reports
// Bulk logical not, the value-returning counterpart of flip(): a sequence's width is its own size(), so unlike the set reading's complement this reads no width as value.
template<class Bits, ownership Own, bool Windowed> [[nodiscard]] constexpr auto operator~(sequence_adaptor<Bits, Own, Windowed> const& lhs) noexcept -> sequence_adaptor<Bits, Own, Windowed> requires (owns(Own)) and requires (sequence_adaptor<Bits, Own, Windowed> c) { c.flip(); } { auto nrv = lhs; nrv.flip(); return nrv; }

// The binary forms of the three above, on an owner alone: a view's copy refers to the very storage it views, so a value returned by one would write through to it.
template<class Bits, ownership Own, bool Windowed> [[nodiscard]] constexpr auto operator&(sequence_adaptor<Bits, Own, Windowed> const& lhs, sequence_adaptor<Bits, Own, Windowed> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Own, Windowed>&>() &= rhs)) -> sequence_adaptor<Bits, Own, Windowed> requires (owns(Own)) and requires (sequence_adaptor<Bits, Own, Windowed> c) { c &= c; } { auto nrv = lhs; nrv &= rhs; return nrv; }
template<class Bits, ownership Own, bool Windowed> [[nodiscard]] constexpr auto operator|(sequence_adaptor<Bits, Own, Windowed> const& lhs, sequence_adaptor<Bits, Own, Windowed> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Own, Windowed>&>() |= rhs)) -> sequence_adaptor<Bits, Own, Windowed> requires (owns(Own)) and requires (sequence_adaptor<Bits, Own, Windowed> c) { c |= c; } { auto nrv = lhs; nrv |= rhs; return nrv; }
template<class Bits, ownership Own, bool Windowed> [[nodiscard]] constexpr auto operator^(sequence_adaptor<Bits, Own, Windowed> const& lhs, sequence_adaptor<Bits, Own, Windowed> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Own, Windowed>&>() ^= rhs)) -> sequence_adaptor<Bits, Own, Windowed> requires (owns(Own)) and requires (sequence_adaptor<Bits, Own, Windowed> c) { c ^= c; } { auto nrv = lhs; nrv ^= rhs; return nrv; }

// NOLINTEND(readability-redundant-parentheses)

// [vector.erasure], over the owner's own erase: the proxies move and swap, so remove_if runs unchanged over the packed bits.
template<class Bits, ownership Own, bool Windowed, class Pred>
constexpr auto erase_if(sequence_adaptor<Bits, Own, Windowed>& c, Pred pred)
        -> sequence_adaptor<Bits, Own, Windowed>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        auto const [first, last] = std::ranges::remove_if(c, pred);
        auto const n = static_cast<sequence_adaptor<Bits, Own, Windowed>::size_type>(last - first);
        c.erase(first, last);
        return n;
}

template<class Bits, ownership Own, bool Windowed, class U = bool>
constexpr auto erase(sequence_adaptor<Bits, Own, Windowed>& c, U const& value)
        -> sequence_adaptor<Bits, Own, Windowed>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        return xstd::erase_if(c, [&](bool x) -> bool { return x == value; });
}

}       // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): the two opt-ins [range.view] and [range.range] invite for a program-defined type.
namespace std::ranges {

// A view is a std::ranges::view outright and borrowed, as set_adaptor's is.
template<class Bits, bool Windowed>
inline constexpr bool enable_view<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed>> = true;

template<class Bits, bool Windowed>
inline constexpr bool enable_borrowed_range<xstd::sequence_adaptor<Bits, xstd::ownership::refers, Windowed>> = true;

}       // namespace std::ranges
// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// The owner hashes as std::vector<bool> does; a view no more than std::span does.
template<class Bits, bool Windowed>
struct hash<xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed>>
{
        [[nodiscard]] constexpr auto operator()(xstd::sequence_adaptor<Bits, xstd::ownership::owns, Windowed> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

}       // namespace std
// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash, so Hash2 takes the hook and not its range overload, which cannot hash the proxy the iterator returns.
namespace boost::container_hash {

template<class Bits, xstd::ownership Own, bool Windowed>
struct is_range<xstd::sequence_adaptor<Bits, Own, Windowed>> : std::false_type {};

}       // namespace boost::container_hash

#endif  // XSTD_BITS_SEQUENCE_ADAPTOR_HPP
