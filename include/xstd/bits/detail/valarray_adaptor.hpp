//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_VALARRAY_ADAPTOR_HPP
#define XSTD_BITS_DETAIL_VALARRAY_ADAPTOR_HPP

#include <xstd/bits/bit_concepts/bit_blocks.hpp>     // bit_blocks
#include <xstd/bits/detail/adapted_bits.hpp>         // adapted_bits
#include <xstd/bits/detail/bit_block_container.hpp>  // bit_block_container_type
#include <xstd/bits/detail/ownership.hpp>            // owned_storage, storage, storage_access, valarray_reading_tag, window
#include <xstd/bits/detail/sequence_adaptor.hpp>     // sequence_adaptor
#include <xstd/bits/from_blocks.hpp>                 // from_blocks, from_blocks_t
#include <xstd/misc/type_traits/empty_base_type.hpp> // empty_base_type
#include <algorithm>                                 // copy
#include <cassert>                                   // assert
#include <cstddef>                                   // size_t
#include <initializer_list>                          // initializer_list
#include <type_traits>                               // is_nothrow_copy_constructible_v, is_nothrow_move_constructible_v, is_nothrow_swappable_v
#include <utility>                                   // in_place, move

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

template<bit_block_container_type Bits, class Derived>
class valarray_adaptor : public adapted_bits<Bits, xstd::empty_base_type<>, Bits::is_structural>
{
        using members_type = adapted_bits<Bits, xstd::empty_base_type<>, Bits::is_structural>;
        using members_type::m_bits;

        using block_type = Bits::block_type;

        // The size is the type's, as std::array's is, where the storage fixes it.
        static constexpr bool is_fixed = Bits::has_static_size;

        static constexpr bool copies_without_throwing = std::is_nothrow_copy_constructible_v<Bits>;

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

        [[nodiscard]] constexpr explicit valarray_adaptor(size_type n)
                requires (not is_fixed)
                : members_type(std::in_place, Bits::check_addressable_width(n))
        {}

        // The value first and the count second, the order [valarray.cons] gives them.
        [[nodiscard]] constexpr valarray_adaptor(value_type const& value, size_type n)
                requires (not is_fixed)
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

        // Not in [valarray]: flat_set's adopting constructor at a run-time size, every bit of the blocks a position.
        [[nodiscard]] constexpr valarray_adaptor(xstd::from_blocks_t, block_container_type blocks) noexcept(std::is_nothrow_move_constructible_v<block_container_type>)
                requires Bits::has_stored_size
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

        // [valarray.access], the proxy where valarray hands out a T&, and the value where it hands out a const T&.
        [[nodiscard]] constexpr auto operator[](size_type n) noexcept
                -> reference
        {
                assert(n < size());
                // Named first: Clang 23's lifetime analysis crashes on a prvalue subscripted through deducing this.
                auto view = elements();
                return view[n];
        }

        [[nodiscard]] constexpr auto operator[](size_type n) const noexcept
                -> value_type
        {
                assert(n < size());
                return m_bits.test(n);
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

#endif // XSTD_BITS_DETAIL_VALARRAY_ADAPTOR_HPP
