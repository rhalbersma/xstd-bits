//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_SEQUENCE_ADAPTOR_HPP
#define XSTD_BITS_SEQUENCE_ADAPTOR_HPP

#include <xstd/bits/detail/allocator_base_type.hpp>      // allocator_base_type
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/hash.hpp>                     // hash_append_bits, std_hash
#include <xstd/bits/detail/intrin.hpp>                   // countr_zero, popcount
#include <xstd/bits/detail/shift.hpp>                    // shl, shr
#include <xstd/bits/detail/random_access.hpp>            // random_access_bit_iterator, random_access_bit_reference
#include <xstd/bits/grid.hpp>                            // adaptor_of
#include <xstd/bits/ownership.hpp>                       // owned_bits_t, owned_storage, owner_of, owner_reading, storage, owns
#include <xstd/bits/tags.hpp>                            // sequence_reading_tag
#include <xstd/misc/concepts/specialization_of.hpp>      // specialization_of_TN
#include <xstd/misc/type_traits/empty_base_type.hpp>     // empty_base_type
#include <boost/container_hash/is_range.hpp>             // is_range
#include <boost/container_hash/is_tuple_like.hpp>        // is_tuple_like
#include <boost/hash2/hash_append.hpp>                   // hash_append_tag
#include <algorithm>                                     // copy, min, remove_if
#include <cassert>                                       // assert
#include <compare>                                       // strong_ordering
#include <concepts>                                      // constructible_from, invocable, same_as, swap, swappable
#include <cstddef>                                       // ptrdiff_t, size_t
#include <format>                                        // format
#include <functional>                                    // hash
#include <initializer_list>                              // initializer_list
#include <iterator>                                      // input_iterator, make_reverse_iterator, reverse_iterator, sentinel_for
#include <limits>                                        // numeric_limits
#include <new>                                           // bad_alloc
#include <optional>                                      // nullopt, optional
#include <ranges>                                        // begin, enable_borrowed_range, enable_view, end, from_range_t, input_range, range_reference_t, size, sized_range, subrange
#include <source_location>                               // source_location
#include <span>                                          // dynamic_extent
#include <stdexcept>                                     // out_of_range
#include <tuple>                                         // tuple_element, tuple_size
#include <type_traits>                                   // conditional_t, false_type, is_invocable_r_v, is_nothrow_swappable_v, remove_const_t, remove_cvref_t, remove_reference_t
#include <utility>                                       // as_const, declval, forward, move, pair

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

// A prvalue from a named parameter: MSVC 17 has no auto(x), which is [P0849R8]'s spelling of this.
template<class T>
[[nodiscard]] constexpr auto decay_copy(T value) noexcept
        -> T
{
        return value;
}

// Continue unless the functor says otherwise: a void functor always continues, a bool one says.
template<class F>
[[nodiscard]] constexpr auto invoke_continues(F& f, bool value)
        -> bool
{
        if constexpr (std::is_invocable_r_v<bool, F&, bool>) {
                return f(decay_copy(value));
        } else {
                f(decay_copy(value));
                return true;
        }
}

// Every position, lowest first, a word at a time: the reload is the inner loop's exit test.
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

// The three aggregates over a window, masked to what it holds: a word at a time, not a test per bit.
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

// The storage's block type: P0634 makes an alias-declaration type-only, where a template argument is not.
template<class Bits>
using block_type_of = std::remove_const_t<Bits>::block_type;

} // namespace detail::sequence

// An owner names its storage's allocator, as std::vector<bool> names its own; a view names none, owning nothing.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, storage Store, bool Windowed, class Derived = void>
class sequence_adaptor;

// The windowed view a span hands back: declared here and defined in its own header, which this one must not include.
template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits>
class bit_subspan;

// A sequence adaptor whose storage holds blocks of the given type: what a blit reads, and nothing else.
template<class S, class Block>
inline constexpr bool blit_source = false;

template<class Bits, storage Store, bool Windowed, class Derived, class Block>
inline constexpr bool blit_source<sequence_adaptor<Bits, Store, Windowed, Derived>, Block> = std::same_as<detail::sequence::block_type_of<Bits>, Block>;

template<specialization_of_TN<detail::bits::contiguous_bit_container> Bits, storage Store, bool Windowed, class Derived>
class sequence_adaptor : public std::conditional_t<owns(Store), detail::bits::allocator_base_type<std::remove_const_t<Bits>>, xstd::empty_base_type<>>
{
        static constexpr bool is_owner = owns(Store);
        static constexpr bool is_window = Windowed;
        static_assert(not(is_owner and is_window), "a window views what another owns");

        using bits_type = std::remove_const_t<Bits>;

        // A width fixed at compile time, which the byte exchange below asks and can_grow is the absence of.
        static constexpr bool has_static_width = (bits_type::extent != std::dynamic_extent);

        // Growth is the owner's over storage that grows: a view must never resize what it does not own.
        static constexpr bool can_grow = is_owner and not has_static_width and requires (bits_type& b, std::size_t n, bool value) { b.resize(n, value); b.push_back(value); b.pop_back(); b.clear(); };

        // The middle column: growth inside a capacity the type carries, whose counterparts disagree on name shape.
        static constexpr bool has_static_capacity = can_grow and bits_type::has_static_capacity;

        // A window is what std::span stores, the pointer's role split in two because bits are not addressable.
        struct window
        {
                Bits* ptr;
                std::size_t offset;
                std::size_t size;
        };

        std::conditional_t<is_owner, Bits, std::conditional_t<is_window, window, Bits*>> m_bits;

        // One accessor: self.m_bits propagates the owner's const, *self.m_bits keeps the view shallow.
        [[nodiscard]] constexpr auto bits(this auto&& self) noexcept
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

        // Where this sequence starts in the storage: zero but for a window, so positions are offset once, here.
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
                : m_bits{ptr, offset, size}
        {}

        // What the accessor hands a given self, const included: the iterator and proxy are spelled over that.
        template<class Self>
        using storage_t = std::remove_reference_t<decltype(std::declval<Self>().bits())>;

        template<class Self>
        using iterator_t = detail::bits::random_access_bit_iterator<storage_t<Self>>;
        template<class Self>
        using reference_t = detail::bits::random_access_bit_reference<storage_t<Self>>;

        // A source the blit can read by block, of this storage's own block type.
        template<class S>
        static constexpr bool blittable = blit_source<S, typename bits_type::block_type>;

        // A storage taking a masked word at any position, asked of Bits so a const window answers no.
        static constexpr bool block_writable = requires (Bits& b, std::size_t pos, bits_type::block_type w) { b.block_at(pos, w, w); };

        // The container needs constraints only the vehicle can name; [class.friend]/3 ignores the void a view passes.
        friend Derived;

        // A sequence view refers into this owner's storage and nothing else does; a set view does not.
        template<specialization_of_TN<detail::bits::contiguous_bit_container> B, storage O, bool W, class D>
        friend class sequence_adaptor;

        // The value under the sequence reading, the owner's alone: a view follows span and hashes no more.
        template<class Provider, class Hash, class Flavor>
        friend constexpr auto tag_invoke(boost::hash2::hash_append_tag const&, Provider const&, Hash& h, Flavor const& f, sequence_adaptor const* v) noexcept
                -> void
                requires is_owner
        {
                detail::bits::hash_append_bits(h, f, v->bits());
        }

public:
        // A vehicle used directly is its own container, which is what a view is.
        using derived_type = std::conditional_t<std::is_void_v<Derived>, sequence_adaptor, Derived>;

        // What a trait asks of this vehicle, every container built on it answering alike.
        using adaptor_type = sequence_adaptor;
        using reads_as = sequence_reading_tag;
        static constexpr bool is_windowed = Windowed;
        using adapted_type = Bits;
        static constexpr bool owns_storage = is_owner;

        // types
        using value_type = bool;
        using pointer = void;
        using const_pointer = pointer;
        using reference = detail::bits::random_access_bit_reference<Bits>;
        using const_reference = detail::bits::random_access_bit_reference<Bits const>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = detail::bits::random_access_bit_iterator<Bits>;
        using const_iterator = detail::bits::random_access_bit_iterator<Bits const>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // construct/copy/destroy: an owner is built as std::array is, or std::vector where the storage grows.
        [[nodiscard]] sequence_adaptor() noexcept
                requires is_owner
        = default;

        [[nodiscard]] constexpr explicit sequence_adaptor(size_type n)
                requires can_grow
                : m_bits(bits_type::check_addressable_width(n))
        {}

        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value)
                requires can_grow
                : m_bits(bits_type::check_addressable_width(n))
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
                : sequence_adaptor(std::ranges::begin(rg), std::ranges::end(rg))
        {}

        [[nodiscard]] constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires can_grow
                : sequence_adaptor(il.begin(), il.end())
        {}

        // std::array's aggregate initialization as a constructor: what is listed leads, the rest stays false.
        constexpr sequence_adaptor(std::initializer_list<value_type> il)
                requires is_owner and (not can_grow)
                : m_bits()
        {
                assert(il.size() <= size());
                std::ranges::copy(il, begin());
        }

        // A field of bits in and out, named rather than spelled as a conversion; never on a window.
        template<class B>
                requires is_owner and bits_type::template
        exchanges_bits<B> [[nodiscard]] static constexpr auto from_bits(B const& b) noexcept
                -> derived_type
        {
                auto result = derived_type();
                result.bits().assign_bits(b);
                return result;
        }

        template<class B>
                requires (not is_window) and bits_type::template
        exchanges_bits<B> [[nodiscard]] constexpr auto to_bits() const noexcept
                -> B
        {
                return bits().template to_bits<B>();
        }

        // [vector.bool]'s allocator arguments, deduced and matched, so a storage without one has no such one.
        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr explicit sequence_adaptor(Alloc const& alloc)
                : m_bits(alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(size_type n, Alloc const& alloc)
                : m_bits(bits_type::check_addressable_width(n), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(size_type n, value_type const& value, Alloc const& alloc)
                : m_bits(bits_type::check_addressable_width(n), alloc)
        {
                if (value) {
                        m_bits.fill(true);
                }
        }

        template<std::input_iterator I, std::sentinel_for<I> S, class Alloc>
                requires can_grow and std::constructible_from<value_type, std::iter_reference_t<I>> and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(I first, S last, Alloc const& alloc)
                : m_bits(alloc)
        {
                for (; first != last; ++first) {
                        m_bits.push_back(static_cast<value_type>(*first));
                }
        }

        template<std::ranges::input_range R, class Alloc>
                requires can_grow and std::constructible_from<value_type, std::ranges::range_reference_t<R>> and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(std::from_range_t, R&& rg, Alloc const& alloc)
                : sequence_adaptor(std::ranges::begin(rg), std::ranges::end(rg), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(sequence_adaptor const& other, Alloc const& alloc)
                : m_bits(other.m_bits, alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(sequence_adaptor&& other, Alloc const& alloc)
                : m_bits(std::move(other.m_bits), alloc)
        {}

        template<class Alloc>
                requires can_grow and std::same_as<Alloc, typename bits_type::allocator_type>
        [[nodiscard]] constexpr sequence_adaptor(std::initializer_list<value_type> il, Alloc const& alloc)
                : sequence_adaptor(il.begin(), il.end(), alloc)
        {}

        [[nodiscard]] constexpr auto get_allocator() const noexcept
                requires is_owner and requires (bits_type const& b) { b.get_allocator(); }
        {
                return m_bits.get_allocator();
        }

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [set] and [vector] return here.
        constexpr auto operator=(std::initializer_list<value_type> il)
                -> derived_type&
                requires can_grow
        {
                assign(il.begin(), il.end());
                return self();
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
                        blit(rg.bits(), rg.offset(), rg.size());
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
                // Through the saturating sum: a wrapped total would answer an insertion with a shorter sequence.
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

        template<class... Args>
                requires can_grow and std::constructible_from<value_type, Args...>
        constexpr auto emplace(const_iterator position, Args&&... args)
                -> iterator
        {
                return insert(position, value_type(std::forward<Args>(args)...));
        }

        constexpr auto erase(const_iterator position)
                -> iterator
                requires can_grow
        {
                // Dereferenceable, which cend() is not: [sequence.reqmts] asks that of the single-position erase.
                assert(position != cend());
                return erase(position, position + 1);
        }

        constexpr auto erase(const_iterator first, const_iterator last)
                -> iterator
                requires can_grow
        {
                // A range, not two positions: the order is what the rebuild's tail subtraction needs.
                assert(first <= last);
                return rebuild(index_of(first), index_of(last), [](sequence_adaptor&) -> void {});
        }

        [[nodiscard]] constexpr explicit sequence_adaptor(Bits& c) noexcept
                requires (not is_owner) and (not is_window)
                : m_bits(&c)
        {}

        // A view over an owner is a view over the storage it wraps; implicit, claiming nothing the owner lacks.
        template<owner_of<Bits, sequence_reading_tag> Owner>
        [[nodiscard]] constexpr explicit(false) sequence_adaptor(Owner& c) noexcept // NOLINT(misc-explicit-constructor)
                requires (not is_owner) and (not is_window)
                : m_bits(&c.m_bits)
        {}

        // [span.sub]'s three, on a view alone: std::array and std::vector have no subviews.
        using subspan_type = bit_subspan<Bits>;

        [[nodiscard]] constexpr auto first(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return {&bits(), offset(), count};
        }

        [[nodiscard]] constexpr auto last(size_type count) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(count <= size());
                return {&bits(), offset() + (size() - count), count};
        }

        [[nodiscard]] constexpr auto subspan(size_type off, size_type count = std::dynamic_extent) const noexcept
                -> subspan_type
                requires (not is_owner)
        {
                assert(off <= size());
                assert(count == std::dynamic_extent or count <= size() - off);
                return {&bits(), offset() + off, count == std::dynamic_extent ? size() - off : count};
        }

        // fill: a masked word at a time over a window of ours, one position at a time over any other.
        constexpr auto fill(this auto&& self, value_type const& u) noexcept
                -> void
                requires (is_window and requires (std::size_t i) { self.bits().assign(i, u); }) or (not is_window and requires { self.bits().fill(u); })
        {
                if constexpr (not is_window) {
                        self.bits().fill(u);
                } else if constexpr (requires { self.bits().set(self.offset(), self.size(), u); }) {
                        self.bits().set(self.offset(), self.size(), u);
                } else {
                        for (auto i = self.offset(), last = self.offset() + self.size(); i < last; ++i) {
                                self.bits().assign(i, u);
                        }
                }
        }

        // The non-member beside it: ranges::swap finds this and never the member.
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
        [[nodiscard]] constexpr auto begin(this auto&& self) noexcept
                -> iterator_t<decltype(self)>
        {
                return {&self.bits(), self.offset()};
        }

        [[nodiscard]] constexpr auto end(this auto&& self) noexcept
                -> iterator_t<decltype(self)>
        {
                return {&self.bits(), self.offset() + self.size()};
        }

        [[nodiscard]] constexpr auto rbegin(this auto&& self) noexcept
        {
                return std::make_reverse_iterator(self.end());
        }

        [[nodiscard]] constexpr auto rend(this auto&& self) noexcept
        {
                return std::make_reverse_iterator(self.begin());
        }

        [[nodiscard]] constexpr auto cbegin() const noexcept
                -> const_iterator
        {
                return {&std::as_const(bits()), offset()};
        }

        [[nodiscard]] constexpr auto cend() const noexcept
                -> const_iterator
        {
                return {&std::as_const(bits()), offset() + size()};
        }

        [[nodiscard]] constexpr auto crbegin() const noexcept
                -> const_reverse_iterator
        {
                return std::make_reverse_iterator(cend());
        }

        [[nodiscard]] constexpr auto crend() const noexcept
                -> const_reverse_iterator
        {
                return std::make_reverse_iterator(cbegin());
        }

        // The sequence reading a word at a time, which the range-for cannot be.
        template<class F>
                requires std::invocable<F&, bool>
        constexpr auto for_each(this auto&& self, F f)
                -> void
        {
                detail::sequence::walk_blocks(self.bits(), self.offset(), self.size(), f);
        }

        // capacity; max_size() is the positions there are to hold, which only a growing one can extend.
        [[nodiscard]] constexpr auto empty() const noexcept
                -> bool
        {
                return size() == 0UZ;
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                if constexpr (is_window) {
                        return m_bits.size;
                } else {
                        return bits().size();
                }
        }

        // [inplace.vector.capacity] makes max_size() a static member, and it can be one: the capacity is the type's.
        [[nodiscard]] static constexpr auto max_size() noexcept
                -> size_type
                requires has_static_capacity
        {
                return bits_type::static_capacity();
        }

        // std::vector<bool>'s answer where this reading can grow, and the width itself where it cannot.
        [[nodiscard]] constexpr auto max_size() const noexcept
                -> size_type
                requires (not has_static_capacity)
        {
                if constexpr (can_grow) {
                        return m_bits.addressable_max_size();
                } else {
                        return size();
                }
        }

        // The sequence reading's aggregates, with the bool [alg.count] and [alg.all.of] give them.
        [[nodiscard]] constexpr auto count(value_type value = true) const noexcept
                -> size_type
        {
                auto const n = count_true();
                return value ? n : size() - n;
        }

        [[nodiscard]] constexpr auto all(value_type value = true) const noexcept
                -> bool
        {
                return value ? all_true() : none_true();
        }

        [[nodiscard]] constexpr auto any(value_type value = true) const noexcept
                -> bool
        {
                return value ? any_true() : not all_true();
        }

        [[nodiscard]] constexpr auto none(value_type value = true) const noexcept
                -> bool
        {
                return value ? none_true() : all_true();
        }

        // std::mismatch's answer over the orderings' machinery: the first differing block and its xor.
        [[nodiscard]] constexpr auto mismatch(sequence_adaptor const& other) const noexcept
                -> size_type
                requires (not is_window) and requires (bits_type const& b) { b.first_difference(b); }
        {
                assert(size() == other.size());
                // A zero width has no blocks to ask about, and answers its own width, which is nought.
                if (empty()) {
                        return 0UZ;
                }
                auto const [index, diff] = bits().first_difference(other.bits());
                using block_type = std::remove_cvref_t<decltype(diff)>;
                constexpr auto digits = static_cast<size_type>(std::numeric_limits<block_type>::digits);
                if (diff == block_type{}) {
                        return size();
                }
                return (index * digits) + detail::bits::countr_zero(diff);
        }

        // Growth, [vector]'s members: the storage computes the ceiling and this reading picks length_error.
        constexpr auto resize(size_type n) -> void
                requires can_grow
        {
                m_bits.resize(bits_type::check_addressable_width(n));
        }

        constexpr auto resize(size_type n, value_type const& value) -> void
                requires can_grow
        {
                m_bits.resize(bits_type::check_addressable_width(n), value);
        }

        constexpr auto clear() noexcept -> void
                requires can_grow
        {
                m_bits.clear();
        }

        constexpr auto pop_back() noexcept -> void
                requires can_grow
        {
                m_bits.pop_back();
        }

        // [inplace.vector.modifiers] returns the reference and [vector.bool] nothing, so the return is deduced.
        constexpr auto push_back(value_type const& value)
                requires can_grow
        {
                m_bits.push_back(value);
                if constexpr (has_static_capacity) {
                        return back();
                }
        }

        // Variadic, as both synopses spell it: value-initialization is an argument list the packing must take.
        template<class... Args>
                requires can_grow and std::constructible_from<value_type, Args...>
        constexpr auto emplace_back(Args&&... args)
                -> reference
        {
                m_bits.push_back(value_type(std::forward<Args>(args)...));
                return back();
        }

        // [inplace.vector.modifiers]'s non-throwing door: a full one answers nullopt, not std::bad_alloc.
        template<class... Args>
                requires has_static_capacity and std::constructible_from<value_type, Args...>
        constexpr auto try_emplace_back(Args&&... args)
                -> std::optional<reference>
        {
                if (size() == capacity()) {
                        return std::nullopt;
                }
                return emplace_back(std::forward<Args>(args)...);
        }

        constexpr auto try_push_back(value_type const& value)
                -> std::optional<reference>
                requires has_static_capacity
        {
                return try_emplace_back(value);
        }

        // The caller has established the room, so this asserts it: size() < capacity() is the precondition.
        template<class... Args>
                requires has_static_capacity and std::constructible_from<value_type, Args...>
        constexpr auto unchecked_emplace_back(Args&&... args)
                -> reference
        {
                assert(size() < capacity());
                return emplace_back(std::forward<Args>(args)...);
        }

        constexpr auto unchecked_push_back(value_type const& value)
                -> reference
                requires has_static_capacity
        {
                return unchecked_emplace_back(value);
        }

        // The middle column's three, static as [inplace.vector.capacity] spells them ([over.load] admits both).
        [[nodiscard]] static constexpr auto capacity() noexcept
                -> size_type
                requires has_static_capacity
        {
                return bits_type::static_capacity();
        }

        // Static, and throwing rather than growing: past the capacity is [inplace.vector.capacity]'s bad_alloc.
        static constexpr auto reserve(size_type n)
                -> void
                requires has_static_capacity
        {
                if (n > capacity()) {
                        throw std::bad_alloc();
                }
        }

        // Static, and a no-op: the capacity cannot shrink, the blocks being the object.
        static constexpr auto shrink_to_fit() noexcept
                -> void
                requires has_static_capacity
        {}

        constexpr auto reserve(size_type n)
                -> void
                requires can_grow and (not has_static_capacity) and requires (bits_type& b) { b.reserve(n); }
        {
                m_bits.reserve(bits_type::check_addressable_width(n));
        }

        [[nodiscard]] constexpr auto capacity() const noexcept
                -> size_type
                requires can_grow and (not has_static_capacity) and requires (bits_type const& b) { b.capacity(); }
        {
                return m_bits.capacity();
        }

        constexpr auto shrink_to_fit()
                -> void
                requires can_grow and (not has_static_capacity) and requires (bits_type& b) { b.shrink_to_fit(); }
        {
                m_bits.shrink_to_fit();
        }

        // element access, [] unchecked and at() throwing as [array] has them.
        [[nodiscard]] constexpr auto operator[](this auto&& self, size_type n) noexcept
                -> reference_t<decltype(self)>
        {
                assert(n < self.size());
                return {&self.bits(), self.offset() + n};
        }

        [[nodiscard]] constexpr auto at(this auto&& self, size_type n)
                -> reference_t<decltype(self)>
        {
                if (n < self.size()) {
                        return {&self.bits(), self.offset() + n};
                }
                throw out_of_range(n, self.size());
        }

        // Both are preconditions in [sequence.reqmts]; each assert is spread so the coverage gate excludes it.
        [[nodiscard]] constexpr auto front(this auto&& self) noexcept
                -> reference_t<decltype(self)>
        {
                assert(not self.empty());
                return {&self.bits(), self.offset()};
        }

        [[nodiscard]] constexpr auto back(this auto&& self) noexcept
                -> reference_t<decltype(self)>
        {
                assert(not self.empty());
                return {&self.bits(), self.offset() + self.size() - 1UZ};
        }

        // The owner's alone, following span: defaulted, the storage being the one member.

        // clang-format off: one line, so "= default;" stays where gcovr's branch exclusion looks for it.
        [[nodiscard]] friend auto operator==(sequence_adaptor const& x, sequence_adaptor const& y) noexcept -> bool requires is_owner = default;
        // clang-format on

        // The storage's entry and nothing else, spelled over bits_type, which MSVC completes eagerly here.
        [[nodiscard]] friend constexpr auto operator<=>(sequence_adaptor const& x, sequence_adaptor const& y) noexcept
                -> std::strong_ordering
                requires is_owner and requires (bits_type const& b) { sequence_lexicographical_compare_three_way(b, b); }
        {
                return sequence_lexicographical_compare_three_way(x.bits(), y.bits());
        }

        // Elementwise logical, as a bitwise operator on a sequence of bools means. Three, not four.
        constexpr auto operator&=(this auto&& self, sequence_adaptor const& other) noexcept -> auto&
                requires (not is_window) and requires { self.bits() &= other.bits(); }
        {
                self.bits() &= other.bits();
                return self;
        }

        constexpr auto operator|=(this auto&& self, sequence_adaptor const& other) noexcept -> auto&
                requires (not is_window) and requires { self.bits() |= other.bits(); }
        {
                self.bits() |= other.bits();
                return self;
        }

        constexpr auto operator^=(this auto&& self, sequence_adaptor const& other) noexcept -> auto&
                requires (not is_window) and requires { self.bits() ^= other.bits(); }
        {
                self.bits() ^= other.bits();
                return self;
        }

        // No shifts: this reading already spells moving elements std::shift_left and std::shift_right.

        // Bulk on a window of ours against a source read by block: a word at a time at either alignment.
        template<class Other>
        constexpr auto operator&=(this auto&& self, Other const& other) noexcept -> auto&
                requires is_window and block_writable and blittable<Other>
        {
                self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a & b); });
                return self;
        }

        template<class Other>
        constexpr auto operator|=(this auto&& self, Other const& other) noexcept -> auto&
                requires is_window and block_writable and blittable<Other>
        {
                self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a | b); });
                return self;
        }

        template<class Other>
        constexpr auto operator^=(this auto&& self, Other const& other) noexcept -> auto&
                requires is_window and block_writable and blittable<Other>
        {
                self.combine(other, [](auto a, auto b) { return static_cast<decltype(a)>(a ^ b); });
                return self;
        }

        // [vector.bool]'s two: flip every bit, and swap two proxies, which the proxies' own swap does.
        constexpr auto flip(this auto&& self) noexcept -> void
                requires (not is_window) and requires { self.bits().flip(); }
        {
                self.bits().flip();
        }

        static constexpr auto swap(reference x, reference y) noexcept
                -> void
        {
                bool const t = x;
                x = y;
                y = t;
        }

private:
        // The container built on this vehicle, which is what its value-returning operators hand back.
        [[nodiscard]] constexpr auto self() noexcept
                -> derived_type&
        {
                return static_cast<derived_type&>(*this);
        }

        // One tier each for the three aggregates above, chosen once by what the target is.
        [[nodiscard]] constexpr auto count_true() const noexcept
                -> size_type
        {
                if constexpr (not is_window) {
                        return bits().count();
                } else {
                        return detail::sequence::count_blocks(bits(), offset(), size());
                }
        }

        [[nodiscard]] constexpr auto any_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return bits().any();
                } else {
                        return detail::sequence::any_blocks(bits(), offset(), size());
                }
        }

        // Its own helper rather than not any_true(), so a storage spelling none() is asked in its words.
        [[nodiscard]] constexpr auto none_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return bits().none();
                } else {
                        return not detail::sequence::any_blocks(bits(), offset(), size());
                }
        }

        // Not count() == size(): a clear position ends it, which is what a word that is not all ones says in one test.
        [[nodiscard]] constexpr auto all_true() const noexcept
                -> bool
        {
                if constexpr (not is_window) {
                        return bits().all();
                } else {
                        return detail::sequence::all_blocks(bits(), offset(), size());
                }
        }

        // The words of this window against another's at its own alignment, masked to what it holds.
        template<class Other, class F>
        constexpr auto combine(this auto&& self, Other const& other, F f) noexcept
                -> void
        {
                using block_type = bits_type::block_type;
                constexpr auto digits = bits_type::bits_per_block;
                assert(self.size() == other.size());
                for (auto k = 0UZ; k < self.size(); k += digits) {
                        auto const mask = detail::sequence::partial_block_mask<block_type>(std::ranges::min(digits, self.size() - k));
                        auto const mine = self.bits().block_at(self.offset() + k);
                        auto const theirs = other.bits().block_at(other.offset() + k);
                        self.bits().block_at(self.offset() + k, f(mine, theirs), mask);
                }
        }

        // Tier one: the source's bits as words at its own alignment, appended a word at a time.
        template<class SBits>
        constexpr auto blit(SBits const& src, size_type first, size_type count)
                -> void
        {
                constexpr auto digits = bits_type::bits_per_block;
                auto const old = size();
                // Through the saturating sum: a wrapped total would answer an append with something shorter.
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
                // Saturating: a sized range's count is the one sum here a caller can wrap on purpose.
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

        // Rebuilt rather than shifted: head, middle, tail, then one swap, so the strong guarantee is free.
        template<class Middle>
        constexpr auto rebuild(size_type pos, size_type tail, Middle&& middle)
                -> iterator
        {
                auto const whole = sequence_adaptor<bits_type const, storage::borrowed, false>(std::as_const(bits()));
                auto tmp = sequence_adaptor();
                tmp.append_range(whole.first(pos));
                middle(tmp);
                tmp.append_range(whole.subspan(tail));
                swap(tmp);
                return begin() + static_cast<difference_type>(pos);
        }

        // The one place a caller's iterator becomes an index, so [sequence.reqmts]'s precondition is said once.
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

// A view deduces the constness of what it views, the way span<T> and span<T const> do.
template<class Bits>
        requires (not requires { typename owned_storage<std::remove_const_t<Bits>>::bits_type; })
sequence_adaptor(Bits&) -> sequence_adaptor<Bits, storage::borrowed, false>;

template<owner_reading<sequence_reading_tag> Owner>
sequence_adaptor(Owner&) -> sequence_adaptor<owned_bits_t<Owner>, storage::borrowed, false>;

// Any container built on the sequence vehicle, the vehicle used directly included.
template<class T>
concept sequence_adaptor_like = requires { typename T::adaptor_type; typename T::reads_as; } and std::same_as<typename T::reads_as, sequence_reading_tag> and std::derived_from<T, typename T::adaptor_type>;

// The owner's side of the protocol above.
template<class Bits, class Derived>
struct owned_storage<sequence_adaptor<Bits, storage::owned, false, Derived>>
{
        using bits_type = Bits;

        // Committed to the sequence reading, so only a sequence view refers into one.
        using reads = sequence_reading_tag;
};

// The sequence cell of the grid, answered where the adaptor it names is defined.
template<class Bits, class Derived>
struct adaptor_of<sequence_reading_tag, Bits, Derived>
{
        using type = sequence_adaptor<Bits, storage::owned, false, Derived>;
};

// NOLINTBEGIN(readability-redundant-parentheses): a call is no primary expression, so the clause needs them.

// Bulk logical not, the value-returning counterpart of flip(): a sequence's width is its own size().
template<class Bits, storage Store, bool Windowed, class Derived>
[[nodiscard]] constexpr auto operator~(sequence_adaptor<Bits, Store, Windowed, Derived> const& lhs) noexcept -> sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type
        requires (owns(Store)) and requires (sequence_adaptor<Bits, Store, Windowed, Derived> c) { c.flip(); }
{
        auto nrv = static_cast<sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type const&>(lhs);
        nrv.flip();
        return nrv;
}

// The binary forms of the three above, on an owner alone: a view's copy refers to the storage it views.
template<class Bits, storage Store, bool Windowed, class Derived>
[[nodiscard]] constexpr auto operator&(sequence_adaptor<Bits, Store, Windowed, Derived> const& lhs, sequence_adaptor<Bits, Store, Windowed, Derived> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Store, Windowed, Derived>&>() &= rhs)) -> sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type
        requires (owns(Store)) and requires (sequence_adaptor<Bits, Store, Windowed, Derived> c) { c &= c; }
{
        auto nrv = static_cast<sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type const&>(lhs);
        nrv &= rhs;
        return nrv;
}

template<class Bits, storage Store, bool Windowed, class Derived>
[[nodiscard]] constexpr auto operator|(sequence_adaptor<Bits, Store, Windowed, Derived> const& lhs, sequence_adaptor<Bits, Store, Windowed, Derived> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Store, Windowed, Derived>&>() |= rhs)) -> sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type
        requires (owns(Store)) and requires (sequence_adaptor<Bits, Store, Windowed, Derived> c) { c |= c; }
{
        auto nrv = static_cast<sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type const&>(lhs);
        nrv |= rhs;
        return nrv;
}

template<class Bits, storage Store, bool Windowed, class Derived>
[[nodiscard]] constexpr auto operator^(sequence_adaptor<Bits, Store, Windowed, Derived> const& lhs, sequence_adaptor<Bits, Store, Windowed, Derived> const& rhs) noexcept(noexcept(std::declval<sequence_adaptor<Bits, Store, Windowed, Derived>&>() ^= rhs)) -> sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type
        requires (owns(Store)) and requires (sequence_adaptor<Bits, Store, Windowed, Derived> c) { c ^= c; }
{
        auto nrv = static_cast<sequence_adaptor<Bits, Store, Windowed, Derived>::derived_type const&>(lhs);
        nrv ^= rhs;
        return nrv;
}

// NOLINTEND(readability-redundant-parentheses)

// [vector.erasure], over the owner's own erase: the proxies move and swap, so remove_if runs unchanged.
template<class Bits, storage Store, bool Windowed, class Derived, class Pred>
constexpr auto erase_if(sequence_adaptor<Bits, Store, Windowed, Derived>& c, Pred pred)
        -> sequence_adaptor<Bits, Store, Windowed, Derived>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        auto const [first, last] = std::ranges::remove_if(c, pred);
        auto const n = static_cast<sequence_adaptor<Bits, Store, Windowed>::size_type>(last - first);
        c.erase(first, last);
        return n;
}

template<class Bits, storage Store, bool Windowed, class Derived, class U = bool>
constexpr auto erase(sequence_adaptor<Bits, Store, Windowed, Derived>& c, U const& value)
        -> sequence_adaptor<Bits, Store, Windowed, Derived>::size_type
        requires requires { c.erase(c.cbegin(), c.cend()); }
{
        return xstd::erase_if(c, [&](bool x) -> bool { return x == value; });
}

// [array]'s tuple interface, the one line of that synopsis a packed bool can answer; static width alone.
template<class Bits, storage Store, bool Windowed>
// NOLINTNEXTLINE(modernize-avoid-c-style-cast): there is no cast here; owns is a function and Store a value.
inline constexpr bool is_static_width_owner = owns(Store) and (not Windowed) and (Bits::extent != std::dynamic_extent);

// Found by ADL, as a program-defined type's get must be: std::get is std's to specialize and this is not std's type.
template<std::size_t I, class Bits, storage Store, bool Windowed, class Derived>
        requires is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
[[nodiscard]] constexpr auto get(sequence_adaptor<Bits, Store, Windowed, Derived>& c) noexcept
{
        return c[I];
}

template<std::size_t I, class Bits, storage Store, bool Windowed, class Derived>
        requires is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
[[nodiscard]] constexpr auto get(sequence_adaptor<Bits, Store, Windowed, Derived> const& c) noexcept
{
        return c[I];
}

// The proxy is returned by value, so the two rvalue overloads forward rather than move.
template<std::size_t I, class Bits, storage Store, bool Windowed, class Derived>
        requires is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
[[nodiscard]] constexpr auto get(sequence_adaptor<Bits, Store, Windowed, Derived>&& c) noexcept
{
        return get<I>(c);
}

template<std::size_t I, class Bits, storage Store, bool Windowed, class Derived>
        requires is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
[[nodiscard]] constexpr auto get(sequence_adaptor<Bits, Store, Windowed, Derived> const&& c) noexcept
{
        return get<I>(c);
}

} // namespace xstd

// NOLINTBEGIN(bugprone-std-namespace-modification): [range.view] and [range.range] invite the opt-in.
namespace std::ranges {

// A view is a std::ranges::view outright and borrowed, as set_adaptor's is.
template<class Bits, bool Windowed>
inline constexpr bool enable_view<xstd::sequence_adaptor<Bits, xstd::storage::borrowed, Windowed>> = true;

template<class Bits, bool Windowed>
inline constexpr bool enable_borrowed_range<xstd::sequence_adaptor<Bits, xstd::storage::borrowed, Windowed>> = true;

} // namespace std::ranges

// NOLINTEND(bugprone-std-namespace-modification)

// NOLINTBEGIN(bugprone-std-namespace-modification)
namespace std {

// [array.tuple]'s three over the static-width owner: tuple_element names the proxy, not bool.
template<class Bits, xstd::storage Store, bool Windowed, class Derived>
        requires xstd::is_static_width_owner<Bits, Store, Windowed>
struct tuple_size<xstd::sequence_adaptor<Bits, Store, Windowed, Derived>>
        : integral_constant<size_t, Bits::extent>
{};

template<size_t I, class Bits, xstd::storage Store, bool Windowed, class Derived>
        requires xstd::is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
struct tuple_element<I, xstd::sequence_adaptor<Bits, Store, Windowed, Derived>>
{
        using type = xstd::sequence_adaptor<Bits, Store, Windowed, Derived>::reference;
};

template<size_t I, class Bits, xstd::storage Store, bool Windowed, class Derived>
        requires xstd::is_static_width_owner<Bits, Store, Windowed> and (I < Bits::extent)
struct tuple_element<I, const xstd::sequence_adaptor<Bits, Store, Windowed, Derived>>
{
        using type = xstd::sequence_adaptor<Bits, Store, Windowed, Derived>::const_reference;
};

// The owner hashes as std::vector<bool> does; a view no more than std::span does.
template<class Bits, bool Windowed, class Derived>
struct hash<xstd::sequence_adaptor<Bits, xstd::storage::owned, Windowed, Derived>>
{
        [[nodiscard]] constexpr auto operator()(xstd::sequence_adaptor<Bits, xstd::storage::owned, Windowed, Derived> const& v) const noexcept
                -> std::size_t
        {
                return xstd::detail::bits::std_hash(v);
        }
};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

// Not a range to ContainerHash and not tuple-like: Hash2 takes the hook, not its range or tuple overload.
namespace boost::container_hash {

template<class Bits, xstd::storage Store, bool Windowed, class Derived>
struct is_range<xstd::sequence_adaptor<Bits, Store, Windowed, Derived>> : std::false_type
{};

template<class Bits, xstd::storage Store, bool Windowed, class Derived>
struct is_tuple_like<xstd::sequence_adaptor<Bits, Store, Windowed, Derived>> : std::false_type
{};

} // namespace boost::container_hash

#endif // XSTD_BITS_SEQUENCE_ADAPTOR_HPP
