//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_SET_HPP
#define XSTD_BITS_BIT_FLAG_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                          // smallest_block_t
#include <xstd/bits/bit_flag_mapping.hpp>                    // bit_flag_mapping
#include <xstd/bits/bit_index_mapping.hpp>                   // sized_bit_index_mapping
#include <xstd/bits/detail/bit_block_container.hpp>          // bit_block_container
#include <xstd/bits/detail/flag_word.hpp>                    // flag_mask, flag_width_v, flag_word_t, from_word, to_word
#include <xstd/bits/detail/ownership.hpp>                    // storage, storage_access
#include <xstd/bits/detail/set_adaptor.hpp>                  // admits_width, set_adaptor
#include <xstd/bits/detail/shift.hpp>                        // shl
#include <xstd/bits/from_blocks.hpp>                         // from_blocks_t
#include <xstd/ints/concepts/bit_mask.hpp>                   // bit_mask
#include <xstd/misc/concepts/container_compatible_range.hpp> // container_compatible_range
#include <boost/container_hash/is_range.hpp>                 // is_range
#include <boost/container_hash/is_tuple_like.hpp>            // is_tuple_like
#include <array>                                             // array
#include <cassert>                                           // assert
#include <concepts>                                          // same_as
#include <cstddef>                                           // size_t
#include <functional>                                        // greater, hash
#include <initializer_list>                                  // initializer_list
#include <iterator>                                          // input_iterator
#include <limits>                                            // numeric_limits
#include <ranges>                                            // from_range_t
#include <type_traits>                                       // false_type
#include <utility>                                           // forward

// A flag type: the set of a bitmask type's one-bit values below N, which converts with the mask itself.
namespace xstd {

// Mask is an enumeration or a std::bitset of one block; its one-bit values are the keys, highest first, as it orders.
template<class Mask, std::size_t N = bits::detail::flag_width_v<Mask>, sized_bit_index_mapping<Mask> KeyMapping = bit_flag_mapping<Mask, N>>
        requires xstd::bit_mask<Mask> and bits::detail::flag_mask<Mask>
class bit_flag_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<smallest_block_t<N>, 1>, N>, bits::detail::storage::owned, bit_flag_set<Mask, N, KeyMapping>, Mask, KeyMapping, std::greater<Mask>> // NOLINT(modernize-use-transparent-functors): a transparent comparator would admit contains(K)
{
        // Descending, so the base's <=> compares the block as a number, as the mask's own order does.
        using base_type  = bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<smallest_block_t<N>, 1>, N>, bits::detail::storage::owned, bit_flag_set<Mask, N, KeyMapping>, Mask, KeyMapping, std::greater<Mask>>; // NOLINT(modernize-use-transparent-functors): as the base clause names it
        using block_type = smallest_block_t<N>;
        using word_type  = bits::detail::flag_word_t<Mask>;

        // One block, every position of it below N a key, so the mapping's universe is the width.
        static_assert(0UZ < N and N <= static_cast<std::size_t>(std::numeric_limits<block_type>::digits));
        static_assert(N <= bits::detail::flag_width_v<Mask>);
        static_assert(bits::detail::set::admits_width<KeyMapping, Mask, N>);

public:
        using typename base_type::value_type;

        // The empty set, which is the zero value [bitmask.types] asks for.
        [[nodiscard]] constexpr bit_flag_set() noexcept
                : base_type()
        {}

        // [set.cons]'s range forms, whose elements are one-bit values.
        template<std::input_iterator InputIterator>
        [[nodiscard]] constexpr bit_flag_set(InputIterator first, InputIterator last)
                : base_type(first, last)
        {}

        template<xstd::container_compatible_range<value_type> R>
        [[nodiscard]] constexpr bit_flag_set(std::from_range_t, R&& rg)
                : base_type(std::from_range, std::forward<R>(rg))
        {}

        // The union of the listed values, so {m} is the conversion from m and {a, b} the set of two one-bit values.
        [[nodiscard]] constexpr bit_flag_set(std::initializer_list<Mask> il) noexcept
        {
                for (auto const& mask : il) {
                        block() = static_cast<block_type>(block() | bits_of(mask));
                }
        }

        // Any value of the mask, as the enumeration it replaces takes it; no bit at or above N.
        [[nodiscard]] constexpr explicit(false) bit_flag_set(Mask const& mask) noexcept // NOLINT(misc-explicit-constructor)
        {
                block() = bits_of(mask);
        }

        // The block as it is, which is bitflags' from_bits_retain; no bit at or above N.
        [[nodiscard]] constexpr bit_flag_set(xstd::from_blocks_t, block_type value) noexcept
        {
                assert(fits(value));
                block() = value;
        }

        [[nodiscard]] constexpr explicit(false) operator Mask() const noexcept // NOLINT(misc-explicit-constructor)
        {
                return bits::detail::from_word<Mask>(static_cast<word_type>(block()));
        }

        // A swap on the base loses to any exact match on this type, so every container declares its own.
        friend constexpr auto swap(bit_flag_set& x, bit_flag_set& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }

        // The set forms, which the mask forms would otherwise hide, over the base as its binary operators call them.
        constexpr auto operator&=(base_type const& other) noexcept
                -> bit_flag_set&
        {
                static_cast<base_type&>(*this) &= other;
                return *this;
        }

        constexpr auto operator|=(base_type const& other) noexcept
                -> bit_flag_set&
        {
                static_cast<base_type&>(*this) |= other;
                return *this;
        }

        constexpr auto operator^=(base_type const& other) noexcept
                -> bit_flag_set&
        {
                static_cast<base_type&>(*this) ^= other;
                return *this;
        }

        constexpr auto operator-=(base_type const& other) noexcept
                -> bit_flag_set&
        {
                static_cast<base_type&>(*this) -= other;
                return *this;
        }

        // Total, as the binary & and - are: the value's bits at or above N meet nothing in this block.
        constexpr auto operator&=(Mask const& other) noexcept
                -> bit_flag_set&
        {
                block() = static_cast<block_type>(block() & low_bits_of(other));
                return *this;
        }

        constexpr auto operator-=(Mask const& other) noexcept
                -> bit_flag_set&
        {
                block() = static_cast<block_type>(block() & static_cast<block_type>(~low_bits_of(other)));
                return *this;
        }

        // These two would bring the value's bits above N into the block, which they must not have.
        constexpr auto operator|=(Mask const& other) noexcept
                -> bit_flag_set&
        {
                block() = static_cast<block_type>(block() | bits_of(other));
                return *this;
        }

        constexpr auto operator^=(Mask const& other) noexcept
                -> bit_flag_set&
        {
                block() = static_cast<block_type>(block() ^ bits_of(other));
                return *this;
        }

        // Templates over the mask's exact type, so that a flag type converting to it is never deduced as one.
        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator==(bit_flag_set const& lhs, M const& rhs) noexcept
                -> bool
        {
                auto const word = bits::detail::to_word(rhs);
                return fits(word) and lhs.block() == static_cast<block_type>(word);
        }

        // The rest in both orders, exact where the base's would take the value as one key; no bit at or above N.
        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator|(bit_flag_set lhs, M const& rhs) noexcept
                -> bit_flag_set
        {
                return lhs |= rhs;
        }

        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator|(M const& lhs, bit_flag_set rhs) noexcept
                -> bit_flag_set
        {
                return rhs |= lhs;
        }

        // Total: truncating the value to N bits is exact, since the flag set has no bit above N for it to keep.
        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator&(bit_flag_set lhs, M const& rhs) noexcept
                -> bit_flag_set
        {
                return lhs &= rhs;
        }

        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator&(M const& lhs, bit_flag_set rhs) noexcept
                -> bit_flag_set
        {
                return rhs &= lhs;
        }

        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator^(bit_flag_set lhs, M const& rhs) noexcept
                -> bit_flag_set
        {
                return lhs ^= rhs;
        }

        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator^(M const& lhs, bit_flag_set rhs) noexcept
                -> bit_flag_set
        {
                return rhs ^= lhs;
        }

        // Total, as & is; the mirror is not, the value's bits above N being what it would keep.
        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator-(bit_flag_set lhs, M const& rhs) noexcept
                -> bit_flag_set
        {
                return lhs -= rhs;
        }

        template<std::same_as<Mask> M>
        [[nodiscard]] friend constexpr auto operator-(M const& lhs, bit_flag_set const& rhs) noexcept
                -> bit_flag_set
        {
                return bit_flag_set(lhs) -= rhs;
        }

private:
        // The one block, every position below N in it.
        [[nodiscard]] constexpr auto block() noexcept
                -> block_type&
        {
                return bits::detail::storage_access::bits(*this)[0];
        }

        [[nodiscard]] constexpr auto block() const noexcept
                -> block_type
        {
                return bits::detail::storage_access::bits(*this)[0];
        }

        // The positions below N, all of a word that can meet a bit of this block.
        static constexpr auto low_mask = static_cast<word_type>(bits::detail::shl(word_type{1}, N - 1UZ) | static_cast<word_type>(bits::detail::shl(word_type{1}, N - 1UZ) - word_type{1}));

        [[nodiscard]] static constexpr auto fits(word_type word) noexcept
                -> bool
        {
                return static_cast<word_type>(word & static_cast<word_type>(~low_mask)) == word_type{};
        }

        [[nodiscard]] static constexpr auto low_bits_of(Mask const& mask) noexcept
                -> block_type
        {
                return static_cast<block_type>(bits::detail::to_word(mask) & low_mask);
        }

        // Where a bit of the value would enter the result, it must have none at or above N.
        [[nodiscard]] static constexpr auto bits_of(Mask const& mask) noexcept
                -> block_type
        {
                assert(fits(bits::detail::to_word(mask)));
                return low_bits_of(mask);
        }
};

} // namespace xstd

namespace boost::container_hash {

// A reading with iterators says it is neither range nor tuple, so Boost hashes it as the value it is.
template<class Mask, std::size_t N, class KeyMapping>
struct is_range<xstd::bit_flag_set<Mask, N, KeyMapping>> : std::false_type
{};

template<class Mask, std::size_t N, class KeyMapping>
struct is_tuple_like<xstd::bit_flag_set<Mask, N, KeyMapping>> : std::false_type
{};

} // namespace boost::container_hash

// NOLINTBEGIN(bugprone-std-namespace-modification): [namespace.std]/2 admits specializing for a program-defined type.

namespace std {

template<class Mask, std::size_t N, class KeyMapping>
struct hash<xstd::bit_flag_set<Mask, N, KeyMapping>> : hash<typename xstd::bit_flag_set<Mask, N, KeyMapping>::adaptor_type>
{};

} // namespace std

// NOLINTEND(bugprone-std-namespace-modification)

#endif // XSTD_BITS_BIT_FLAG_SET_HPP
