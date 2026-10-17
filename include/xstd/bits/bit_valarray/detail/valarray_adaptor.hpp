//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_ADAPTOR_HPP
#define XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_ADAPTOR_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>            // bit_blocks
#include <xstd/bits/bit_valarray/bit_gslice.hpp>            // bit_gslice
#include <xstd/bits/bit_valarray/bit_gslice_array.hpp>      // bit_gslice_array
#include <xstd/bits/bit_valarray/bit_indirect_array.hpp>    // bit_indirect_array
#include <xstd/bits/bit_valarray/bit_mask_array.hpp>        // bit_mask_array
#include <xstd/bits/bit_valarray/bit_slice.hpp>             // bit_slice
#include <xstd/bits/bit_valarray/bit_slice_array.hpp>       // bit_slice_array
#include <xstd/bits/bit_valarray/detail/valarray_proxy.hpp> // gslice_selection, indirect_selection, mask_selection, proxy_access, slice_selection, valarray_owner_of
#include <xstd/bits/detail/adapted_bits.hpp>                // adapted_bits
#include <xstd/bits/detail/assert.hpp>                      // XSTD_BITS_ASSERT
#include <xstd/bits/detail/bit_block_container.hpp>         // bit_block_container_type
#include <xstd/bits/detail/borrowed_bits.hpp>               // borrowed_bits
#include <xstd/bits/detail/ownership.hpp>                   // owned_storage, storage, storage_access, valarray_reading_tag, window
#include <xstd/bits/detail/sequence_adaptor.hpp>            // sequence_adaptor
#include <xstd/bits/from_blocks.hpp>                        // from_blocks, from_blocks_t
#include <xstd/misc/type_traits/empty_base_type.hpp>        // empty_base_type
#include <algorithm>                                        // copy
#include <cassert>                                          // assert
#include <cstddef>                                          // size_t
#include <initializer_list>                                 // initializer_list
#include <span>                                             // span
#include <type_traits>                                      // conditional_t, is_nothrow_copy_constructible_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v
#include <utility>                                          // in_place, move
#include <valarray>                                         // gslice, slice, valarray

// The valarray reading, [valarray] at bool over a bit_block_container it owns: every operator acts elementwise.
namespace xstd::bits::detail {

namespace valarray {

template<class Block>
[[nodiscard]] constexpr auto inverted(Block a) noexcept
        -> Block
{
        return static_cast<Block>(~a);
}

// Each elementwise operator as the bitwise expression that answers it for a whole block of positions at once.
inline constexpr auto bit_and = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(a & b); };

inline constexpr auto bit_or = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(a | b); };

inline constexpr auto bit_xor = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(a ^ b); };

inline constexpr auto equal_to = []<class Block>(Block a, Block b) noexcept -> Block { return inverted(static_cast<Block>(a ^ b)); };

inline constexpr auto less = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(inverted(a) & b); };

inline constexpr auto greater = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(a & inverted(b)); };

inline constexpr auto less_equal = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(inverted(a) | b); };

inline constexpr auto greater_equal = []<class Block>(Block a, Block b) noexcept -> Block { return static_cast<Block>(a | inverted(b)); };

// The count a shift or a rotation by n moves, whatever its sign; the subtraction is modular, so INT_MIN is exact.
[[nodiscard]] constexpr auto magnitude(int n) noexcept
        -> std::size_t
{
        return n < 0 ? 0UZ - static_cast<std::size_t>(n) : static_cast<std::size_t>(n);
}

} // namespace valarray

// The owner a const subscript gathers into: an owner of run-time size its own type, and a fixed one specializes it.
template<class Owner>
struct gathered
{
        using type = Owner;
};

template<class Owner>
using gathered_t = gathered<Owner>::type;

template<bit_block_container_type Bits, class Derived>
class valarray_adaptor : public adapted_bits<Bits, xstd::empty_base_type<>, Bits::is_structural>
{
        using members_type = adapted_bits<Bits, xstd::empty_base_type<>, Bits::is_structural>;
        using members_type::m_bits;

        using block_type = Bits::block_type;

        // The size is the type's, as std::array's is, where the storage fixes it.
        static constexpr bool is_fixed = Bits::has_static_size;

        static constexpr bool copies_without_throwing = std::is_nothrow_copy_constructible_v<Bits>;

        // No caller can make one, so a constructor taking it cannot be called.
        class unreachable
        {
                unreachable() = default;
        };

        // Types rather than constraints, since MSVC drops an inherited constructor's requires-clause candidate.
        using count_type   = std::conditional_t<is_fixed, unreachable, std::size_t>;
        using adopted_type = std::conditional_t<Bits::has_stored_size, typename Bits::block_container_type, unreachable>;

        // The elements as the sequence reading hands them out: a whole view, whose proxies and iterators are these.
        using elements_type       = sequence_adaptor<Bits, storage::borrowed, window::all>;
        using const_elements_type = sequence_adaptor<Bits const, storage::borrowed, window::all>;

        [[nodiscard]] constexpr auto elements() noexcept
                -> elements_type
        {
                return elements_type(m_bits);
        }

        [[nodiscard]] constexpr auto elements() const noexcept
                -> const_elements_type
        {
                return const_elements_type(m_bits);
        }

        [[nodiscard]] constexpr auto bits() noexcept
                -> Bits&
        {
                return m_bits;
        }

        [[nodiscard]] constexpr auto bits() const noexcept
                -> Bits const&
        {
                return m_bits;
        }

        [[nodiscard]] constexpr auto self() noexcept
                -> Derived&
        {
                return static_cast<Derived&>(*this);
        }

        [[nodiscard]] constexpr auto self() const noexcept
                -> Derived const&
        {
                return static_cast<Derived const&>(*this);
        }

        // The four proxies borrow the blocks, as std::slice_array refers to its std::valarray.
        [[nodiscard]] constexpr auto borrowed() noexcept
                -> std::span<block_type>
        {
                return m_bits.blocks();
        }

        // Only a constructor delegating from a proxy can name it.
        struct gathering
        {};

        // Sized to what the proxy selects, which at a fixed size must be the type's own.
        template<class Proxy>
        [[nodiscard]] constexpr valarray_adaptor(gathering, Proxy const& a)
        {
                auto const n = proxy_access::size(a);
                if constexpr (is_fixed) {
                        assert(n == size());
                } else {
                        m_bits.resize(Bits::check_addressable_width(n));
                }
                auto dst = borrowed_bits<block_type>(borrowed());
                proxy_access::gather(a, dst);
        }

        // A const subscript's elements, in a valarray of their own of the owner gathered_t names.
        template<class Selection>
        [[nodiscard]] constexpr auto gather(Selection const& selection) const
        {
                auto result = gathered_t<Derived>(selection.size());
                auto dst    = borrowed_bits<block_type>(storage_access::bits(result).blocks());
                // Read through the view a proxy writes through, so every owner over one block runs one gather.
                auto const blocks = m_bits.blocks();
                auto const src    = borrowed_bits<block_type>(std::span(const_cast<block_type*>(blocks.data()), blocks.size())); // NOLINT(cppcoreguidelines-pro-type-const-cast): the view is only read.
                selection.gather(src, dst);
                return result;
        }

        // The free functions over every reading, bit_convert among them, reach the storage through this one door.
        friend struct storage_access;

        // A sequence view over this owner refers into its storage, as std::span<bool> does into a std::valarray<bool>.
        template<bit_block_container_type, storage, window, class, std::size_t>
        friend class sequence_adaptor;

public:
        // What a trait asks of this vehicle, every container built on it answering alike.
        using adaptor_type = valarray_adaptor;
        using reads_as     = valarray_reading_tag;

        static constexpr bool is_windowed = false;

        // types: [valarray]'s value_type, and the proxy that stands in for its T&.
        using value_type           = bool;
        using reference            = elements_type::reference;
        using size_type            = std::size_t;
        using block_container_type = Bits::block_container_type;

        // [valarray.cons], the sizes taken at run time where the storage has one.
        [[nodiscard]] valarray_adaptor() = default;

        [[nodiscard]] constexpr explicit valarray_adaptor(count_type n)
                : members_type(std::in_place, Bits::check_addressable_width(n))
        {}

        // The value first and the count second, the order [valarray.cons] gives them.
        [[nodiscard]] constexpr valarray_adaptor(value_type const& value, count_type n)
                : valarray_adaptor(n)
        {
                m_bits.fill(value);
        }

        // At a fixed size, std::array's aggregate initialization: what is listed leads, the rest stays false.
        [[nodiscard]] constexpr valarray_adaptor(std::initializer_list<value_type> il)
        {
                if constexpr (is_fixed) {
                        assert(il.size() <= size());
                } else {
                        m_bits.resize(Bits::check_addressable_width(il.size()));
                }
                std::ranges::copy(il, elements().begin());
        }

        // Not in [valarray]: a built-in array of bools at a fixed size, position by position, as bit_array takes one.
        template<std::size_t M>
                requires is_fixed and (M == Bits::extent)
        [[nodiscard]] constexpr explicit valarray_adaptor(value_type const (&a)[M]) noexcept // NOLINT(modernize-avoid-c-arrays): a built-in array is what it converts.
        {
                std::ranges::copy(a, elements().begin());
        }

        // [valarray.cons]: n bools read from p, where n is the type's own at a fixed size.
        [[nodiscard]] constexpr valarray_adaptor(value_type const* p, size_type n)
        {
                if constexpr (is_fixed) {
                        assert(n == size());
                } else {
                        m_bits.resize(Bits::check_addressable_width(n));
                }
                std::ranges::copy(std::span(p, n), elements().begin());
        }

        // [valarray.cons]: as many elements as a proxy selects, which at a fixed size must be the type's own size.
        [[nodiscard]] constexpr explicit(false) valarray_adaptor(xstd::bit_slice_array<block_type> const& a) // NOLINT(misc-explicit-constructor)
                : valarray_adaptor(gathering(), a)
        {}

        [[nodiscard]] constexpr explicit(false) valarray_adaptor(xstd::bit_gslice_array<block_type> const& a) // NOLINT(misc-explicit-constructor)
                : valarray_adaptor(gathering(), a)
        {}

        [[nodiscard]] constexpr explicit(false) valarray_adaptor(xstd::bit_mask_array<block_type> const& a) // NOLINT(misc-explicit-constructor)
                : valarray_adaptor(gathering(), a)
        {}

        [[nodiscard]] constexpr explicit(false) valarray_adaptor(xstd::bit_indirect_array<block_type> const& a) // NOLINT(misc-explicit-constructor)
                : valarray_adaptor(gathering(), a)
        {}

        // Not in [valarray]: flat_set's adopting constructor at a run-time size, every bit of the blocks a position.
        [[nodiscard]] constexpr valarray_adaptor(xstd::from_blocks_t, adopted_type blocks) noexcept(std::is_nothrow_move_constructible_v<adopted_type>)
                : members_type(std::in_place, xstd::from_blocks, std::move(blocks))
        {}

        // Not in [valarray]: blocks that are bit storage, read as this array's bools at a fixed size.
        template<class OtherBits>
                requires xstd::bit_blocks<OtherBits> and Bits::template
        exchanges_bits<OtherBits> [[nodiscard]] constexpr valarray_adaptor(xstd::from_blocks_t, OtherBits const& b) noexcept
        {
                m_bits.assign_bits(b);
        }

        // [valarray.assign]: a scalar assigned to every element, and a list as the array it constructs.
        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(value_type const& value) noexcept
                -> Derived&
        {
                m_bits.fill(value);
                return self();
        }

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(std::initializer_list<value_type> il)
                -> Derived&
        {
                return self() = Derived(il);
        }

        // [valarray.assign]: a proxy's elements, gathered before any is assigned, so it may select from this array.
        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(xstd::bit_slice_array<block_type> const& a)
                -> Derived&
        {
                return self() = Derived(a);
        }

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(xstd::bit_gslice_array<block_type> const& a)
                -> Derived&
        {
                return self() = Derived(a);
        }

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(xstd::bit_mask_array<block_type> const& a)
                -> Derived&
        {
                return self() = Derived(a);
        }

        // NOLINTNEXTLINE(misc-unconventional-assign-operator): the container is what [valarray.assign] returns.
        constexpr auto operator=(xstd::bit_indirect_array<block_type> const& a)
                -> Derived&
        {
                return self() = Derived(a);
        }

        // [valarray.access], the proxy where valarray hands out a T&, and the value where it hands out a const T&.
        [[nodiscard]] constexpr auto operator[](size_type n) noexcept
                -> reference
        {
                XSTD_BITS_ASSERT(n < size());
                // Named first: Clang 23's lifetime analysis crashes on a prvalue subscripted through deducing this.
                auto view = elements();
                return view[n];
        }

        [[nodiscard]] constexpr auto operator[](size_type n) const noexcept
                -> value_type
        {
                XSTD_BITS_ASSERT(n < size());
                return m_bits.test(n);
        }

        // [valarray.sub]: a gather into a valarray of its own where the array is const, else a proxy that refers to it.
        [[nodiscard]] auto operator[](std::slice s) const
        {
                return (*this)[xstd::bit_slice(s)];
        }

        [[nodiscard]] auto operator[](std::slice s)
                -> xstd::bit_slice_array<block_type>
        {
                return (*this)[xstd::bit_slice(s)];
        }

        // Not in [valarray]: a bit_slice, which a constant expression can make, as it cannot a std::slice.
        [[nodiscard]] constexpr auto operator[](xstd::bit_slice s) const
        {
                assert(s.size() == 0UZ or s.start() + ((s.size() - 1UZ) * s.stride()) < size());
                return gather(slice_selection(s));
        }

        [[nodiscard]] constexpr auto operator[](xstd::bit_slice s)
                -> xstd::bit_slice_array<block_type>
        {
                assert(s.size() == 0UZ or s.start() + ((s.size() - 1UZ) * s.stride()) < size());
                return proxy_access::make<xstd::bit_slice_array<block_type>>(borrowed(), slice_selection(s));
        }

        [[nodiscard]] auto operator[](std::gslice const& s) const
        {
                return (*this)[xstd::bit_gslice(s)];
        }

        [[nodiscard]] auto operator[](std::gslice const& s)
                -> xstd::bit_gslice_array<block_type>
        {
                return (*this)[xstd::bit_gslice(s)];
        }

        // Not in [valarray]: a bit_gslice, whose positions are worked out once for every subscript that reuses it.
        [[nodiscard]] auto operator[](xstd::bit_gslice const& s) const
        {
                auto const selection = gslice_selection(s);
                assert(selection.within(size()));
                return gather(selection);
        }

        [[nodiscard]] auto operator[](xstd::bit_gslice const& s)
                -> xstd::bit_gslice_array<block_type>
        {
                auto selection = gslice_selection(s);
                assert(selection.within(size()));
                return proxy_access::make<xstd::bit_gslice_array<block_type>>(borrowed(), std::move(selection));
        }

        // Any valarray of one size and block is a mask, which a proxy refers to and does not copy: it must outlive it.
        template<class Mask>
                requires valarray_owner_of<Mask, block_type>
        [[nodiscard]] constexpr auto operator[](Mask const& mask) const
        {
                assert(mask.size() == size());
                return gather(mask_selection<block_type>(storage_access::bits(mask).blocks()));
        }

        template<class Mask>
                requires valarray_owner_of<Mask, block_type>
        [[nodiscard]] constexpr auto operator[](Mask const& mask)
                -> xstd::bit_mask_array<block_type>
        {
                assert(mask.size() == size());
                return proxy_access::make<xstd::bit_mask_array<block_type>>(borrowed(), mask_selection<block_type>(storage_access::bits(mask).blocks()));
        }

        [[nodiscard]] auto operator[](std::valarray<std::size_t> const& indices) const
        {
                auto const selection = indirect_selection(indices);
                assert(selection.within(size()));
                return gather(selection);
        }

        [[nodiscard]] auto operator[](std::valarray<std::size_t> const& indices)
                -> xstd::bit_indirect_array<block_type>
        {
                auto const selection = indirect_selection(indices);
                assert(selection.within(size()));
                return proxy_access::make<xstd::bit_indirect_array<block_type>>(borrowed(), selection);
        }

        // [valarray.members]
        constexpr auto swap(Derived& other) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
        {
                m_bits.swap(other.m_bits);
        }

        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return m_bits.size();
        }

        // The sum of bools is their disjunction; an empty array's is false, which [valarray.members] leaves undefined.
        [[nodiscard]] constexpr auto sum() const noexcept
                -> value_type
        {
                return m_bits.any();
        }

        // The least of bools is their conjunction; an empty array's is true, the identity of and.
        [[nodiscard]] constexpr auto min() const noexcept
                -> value_type
        {
                return m_bits.all();
        }

        [[nodiscard]] constexpr auto max() const noexcept
                -> value_type
        {
                return m_bits.any();
        }

        // Element i takes element i + n, false where that lies outside: n > 0 moves the elements towards the front.
        [[nodiscard]] constexpr auto shift(int n) const noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv         = self();
                auto const count = valarray::magnitude(n);
                if (count >= size()) {
                        nrv.m_bits.fill(false);
                } else if (n > 0) {
                        nrv.m_bits >>= count;
                } else {
                        nrv.m_bits <<= count;
                }
                return nrv;
        }

        // Element i takes element (i + n) mod size(); a negative n turns the other way, and an empty array not at all.
        [[nodiscard]] constexpr auto cshift(int n) const noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv = self();
                if (n >= 0) {
                        nrv.m_bits.rotate(static_cast<std::size_t>(n));
                } else if (size() != 0UZ) {
                        nrv.m_bits.rotate(size() - (valarray::magnitude(n) % size()));
                }
                return nrv;
        }

        // A function of one bool is one of four, so it is asked of false and of true once each, then applied blockwise.
        [[nodiscard]] constexpr auto apply(value_type func(value_type)) const
                -> Derived
        {
                return applied(func(false), func(true));
        }

        [[nodiscard]] constexpr auto apply(value_type func(value_type const&)) const
                -> Derived
        {
                return applied(func(false), func(true));
        }

        constexpr auto resize(size_type n, value_type value = false)
                -> void
                requires (not is_fixed)
        {
                // The new elements are made with the value, then the old ones take it.
                m_bits.resize(Bits::check_addressable_width(n), value);
                m_bits.fill(value);
        }

        // [valarray.unary] at bool: logical not, which ~ spells too; valarray<bool>'s ~ promotes, and is never false.
        [[nodiscard]] constexpr auto operator!() const noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv = self();
                nrv.m_bits.flip();
                return nrv;
        }

        [[nodiscard]] constexpr auto operator~() const noexcept(copies_without_throwing)
                -> Derived
        {
                return not *this;
        }

        // [valarray.cassign]'s logical three, against an array of the same size and against a scalar.
        constexpr auto operator&=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits.transform(other.m_bits, valarray::bit_and);
                return self();
        }

        constexpr auto operator|=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits.transform(other.m_bits, valarray::bit_or);
                return self();
        }

        constexpr auto operator^=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits.transform(other.m_bits, valarray::bit_xor);
                return self();
        }

        constexpr auto operator&=(value_type const& value) noexcept
                -> Derived&
        {
                if (not value) {
                        m_bits.fill(false);
                }
                return self();
        }

        constexpr auto operator|=(value_type const& value) noexcept
                -> Derived&
        {
                if (value) {
                        m_bits.fill(true);
                }
                return self();
        }

        constexpr auto operator^=(value_type const& value) noexcept
                -> Derived&
        {
                if (value) {
                        m_bits.flip();
                }
                return self();
        }

        // Not [valarray.cassign]'s elementwise shifts, which promote bools: std::bitset's, position i to i + n for <<.
        constexpr auto operator<<=(size_type n) noexcept
                -> Derived&
        {
                if (n < size()) {
                        m_bits <<= n;
                } else {
                        m_bits.fill(false);
                }
                return self();
        }

        constexpr auto operator>>=(size_type n) noexcept
                -> Derived&
        {
                if (n < size()) {
                        m_bits >>= n;
                } else {
                        m_bits.fill(false);
                }
                return self();
        }

        [[nodiscard]] friend constexpr auto operator<<(Derived const& x, size_type n) noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv = x;
                nrv <<= n;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator>>(Derived const& x, size_type n) noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv = x;
                nrv >>= n;
                return nrv;
        }

        // [valarray.special]
        friend constexpr auto swap(Derived& x, Derived& y) noexcept(std::is_nothrow_swappable_v<Bits>)
                -> void
        {
                x.swap(y);
        }

        // [valarray.range]: non-members, valarray having no member begin or end.
        [[nodiscard]] friend constexpr auto begin(Derived& v) noexcept
        {
                return v.elements().begin();
        }

        [[nodiscard]] friend constexpr auto begin(Derived const& v) noexcept
        {
                return v.elements().begin();
        }

        [[nodiscard]] friend constexpr auto end(Derived& v) noexcept
        {
                return v.elements().end();
        }

        [[nodiscard]] friend constexpr auto end(Derived const& v) noexcept
        {
                return v.elements().end();
        }

        // [valarray.binary]'s logical five and [valarray.comparison]'s six, over two arrays or an array and a scalar.
        [[nodiscard]] friend constexpr auto operator&(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator&(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator&(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator|(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        [[nodiscard]] friend constexpr auto operator|(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        [[nodiscard]] friend constexpr auto operator|(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        [[nodiscard]] friend constexpr auto operator^(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        [[nodiscard]] friend constexpr auto operator^(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        [[nodiscard]] friend constexpr auto operator^(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        // Both operands are evaluated, as for every overloaded && and ||: at bool they are & and |.
        [[nodiscard]] friend constexpr auto operator&&(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator&&(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator&&(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_and);
        }

        [[nodiscard]] friend constexpr auto operator||(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        [[nodiscard]] friend constexpr auto operator||(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        [[nodiscard]] friend constexpr auto operator||(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_or);
        }

        // An array of answers, not one: so the type is not equality_comparable, as std::valarray is not.
        [[nodiscard]] friend constexpr auto operator==(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::equal_to);
        }

        [[nodiscard]] friend constexpr auto operator==(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::equal_to);
        }

        [[nodiscard]] friend constexpr auto operator==(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::equal_to);
        }

        [[nodiscard]] friend constexpr auto operator!=(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        [[nodiscard]] friend constexpr auto operator!=(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        [[nodiscard]] friend constexpr auto operator!=(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::bit_xor);
        }

        [[nodiscard]] friend constexpr auto operator<(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less);
        }

        [[nodiscard]] friend constexpr auto operator<(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less);
        }

        [[nodiscard]] friend constexpr auto operator<(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less);
        }

        [[nodiscard]] friend constexpr auto operator>(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater);
        }

        [[nodiscard]] friend constexpr auto operator>(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater);
        }

        [[nodiscard]] friend constexpr auto operator>(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater);
        }

        [[nodiscard]] friend constexpr auto operator<=(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less_equal);
        }

        [[nodiscard]] friend constexpr auto operator<=(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less_equal);
        }

        [[nodiscard]] friend constexpr auto operator<=(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::less_equal);
        }

        [[nodiscard]] friend constexpr auto operator>=(Derived const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater_equal);
        }

        [[nodiscard]] friend constexpr auto operator>=(Derived const& x, value_type const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater_equal);
        }

        [[nodiscard]] friend constexpr auto operator>=(value_type const& x, Derived const& y) noexcept(copies_without_throwing)
                -> Derived
        {
                return zip(x, y, valarray::greater_equal);
        }

private:
        // Where func(false) and func(true) agree the answer is that constant, and otherwise the identity or not.
        [[nodiscard]] constexpr auto applied(value_type at_false, value_type at_true) const noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv = self();
                if (at_false == at_true) {
                        nrv.m_bits.fill(at_false);
                } else if (at_false) {
                        nrv.m_bits.flip();
                }
                return nrv;
        }

        // A scalar operand as a block holding it at every position.
        [[nodiscard]] static constexpr auto splat(value_type value) noexcept
                -> block_type
        {
                return value ? valarray::inverted(block_type{}) : block_type{};
        }

        template<class F>
        [[nodiscard]] static constexpr auto zip(Derived const& x, Derived const& y, F f) noexcept(copies_without_throwing)
                -> Derived
        {
                assert(x.size() == y.size());
                auto nrv = x;
                nrv.m_bits.transform(y.m_bits, f);
                return nrv;
        }

        template<class F>
        [[nodiscard]] static constexpr auto zip(Derived const& x, value_type y, F f) noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv     = x;
                auto const b = splat(y);
                nrv.m_bits.transform([&](block_type a) noexcept -> block_type { return f(a, b); });
                return nrv;
        }

        template<class F>
        [[nodiscard]] static constexpr auto zip(value_type x, Derived const& y, F f) noexcept(copies_without_throwing)
                -> Derived
        {
                auto nrv     = y;
                auto const a = splat(x);
                nrv.m_bits.transform([&](block_type b) noexcept -> block_type { return f(a, b); });
                return nrv;
        }
};

// What a valarray owner wraps, so that a sequence view over it names the same storage.
template<class Bits, class Derived>
struct owned_storage<valarray_adaptor<Bits, Derived>>
{
        using bits_type = Bits;
};

} // namespace xstd::bits::detail

#endif // XSTD_BITS_BIT_VALARRAY_DETAIL_VALARRAY_ADAPTOR_HPP
