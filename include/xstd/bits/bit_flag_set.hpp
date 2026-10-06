//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_SET_HPP
#define XSTD_BITS_BIT_FLAG_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                 // bit_blocks_extent_v
#include <xstd/bits/bit_index_mapping.hpp>          // bit_index_mapping, sized_bit_index_mapping
#include <xstd/bits/bit_key_mapping.hpp>            // bit_key_mapping
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container, num_blocks_v
#include <xstd/bits/detail/mask_word.hpp>           // low_mask_bits, mask_fits, mask_width, mask_word, to_mask
#include <xstd/bits/detail/ownership.hpp>           // storage, storage_access
#include <xstd/bits/detail/set_adaptor.hpp>         // set_adaptor
#include <xstd/bits/detail/shift.hpp>               // shl, shr
#include <xstd/bits/from_blocks.hpp>                // from_blocks_t
#include <xstd/ints/concepts/unsigned_integer.hpp>  // unsigned_integer
#include <array>                                    // array
#include <cassert>                                  // assert
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <functional>                               // greater
#include <type_traits>                              // conditional_t, is_void_v

// A base for flag types: the set reading over one block, spelled as the bitmask enumeration it replaces.
namespace xstd {

// BitMask is the flag type, itself a bit_mask; Key names one flag, a rank or a one-bit value under bit_flag_mapping.
template<class BitMask, class Key, xstd::unsigned_integer Block, std::size_t N = bit_blocks_extent_v<Block>, bit_index_mapping<Key> KeyMapping = bit_key_mapping<Key>, class Interop = void>
        requires (std::is_void_v<Interop> or bits::detail::mask_word<Interop>)                                                                                                                                                  // Interop, unless void, is a bit_mask whose bits can be read: an enumeration, an unsigned integer or a std::bitset.
class bit_flag_set : public bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, BitMask, Key, KeyMapping, std::greater<Key>> // NOLINT(modernize-use-transparent-functors): a transparent comparator would admit contains(K)
{
        // Descending, so the base's <=> compares blocks as numbers, as the enumeration it replaces does.
        using base_type = bits::detail::set_adaptor<bits::detail::bit_block_container<std::array<Block, bits::detail::num_blocks_v<Block, N>>, N>, bits::detail::storage::owned, BitMask, Key, KeyMapping, std::greater<Key>>; // NOLINT(modernize-use-transparent-functors): as the base clause names it

        static_assert(N <= bit_blocks_extent_v<Block>);

        // The keys are the universe the mapping closes, else every position of the block.
        [[nodiscard]] static consteval auto num_keys() noexcept
                -> std::size_t
        {
                if constexpr (sized_bit_index_mapping<KeyMapping, Key>) {
                        return KeyMapping::size;
                } else {
                        return N;
                }
        }

        static_assert(num_keys() <= N);

        static constexpr bool has_interop = not std::is_void_v<Interop>;

        // The interop mask holds every position of the block, so the block converts to it whole.
        [[nodiscard]] static consteval auto interop_holds_width() noexcept
                -> bool
        {
                if constexpr (has_interop) {
                        return N <= bits::detail::mask_width<Interop>();
                } else {
                        return true;
                }
        }

        static_assert(interop_holds_width());

        // What a conversion with no interop mask takes and gives: a type nothing else names.
        struct no_interop
        {};

        using interop_param = std::conditional_t<has_interop, Interop, no_interop>;

public:
        using block_type   = Block;
        using interop_type = Interop;

        // p[k]: read as contains(k), written as insert or erase; nested, so ADL on it searches no user's namespace.
        class reference
        {
                Block* m_ptr;
                std::size_t m_pos;

                friend bit_flag_set;

                [[nodiscard]] constexpr reference(Block* ptr, std::size_t pos) noexcept
                        : m_ptr(ptr)
                        , m_pos(pos)
                {}

        public:
                using value_type = bool;

                // Said out loud: the assignments below are user-provided, which deprecates the implicit copy.
                reference(reference const&) = default;

                // The one conversion: p[k] == true and if (p[k]) are the built-in bool ones through it.
                [[nodiscard]] constexpr explicit(false) operator value_type() const noexcept // NOLINT(misc-explicit-constructor)
                {
                        return (bits::detail::shr(*m_ptr, m_pos) & Block{1}) != Block{};
                }

                constexpr auto operator=(value_type value) const noexcept // NOLINT(misc-unconventional-assign-operator)
                        -> reference const&
                {
                        auto const bit    = bits::detail::shl(Block{1}, m_pos);
                        auto const others = static_cast<Block>(*m_ptr & static_cast<Block>(~bit));
                        *m_ptr            = value ? static_cast<Block>(others | bit) : others;
                        return *this;
                }

                // Assigns the bit, not the proxy: p[a] = p[b] copies a flag rather than rebinding a handle.
                constexpr auto operator=(reference const& other) const noexcept // NOLINT(misc-unconventional-assign-operator,bugprone-unhandled-self-assignment)
                        -> reference const&
                {
                        return *this = static_cast<value_type>(other);
                }
        };

        // The empty set, which is the zero value [bitmask.types] asks for.
        [[nodiscard]] bit_flag_set() = default;

        // One flag, explicit so that a key never meets a flag type through a conversion.
        [[nodiscard]] constexpr explicit bit_flag_set(Key key) noexcept
        {
                block() = bits::detail::shl(Block{1}, position_of(key));
        }

        // The block as it is, unnamed positions included, which is bitflags' from_bits_retain; no bit at or above N.
        [[nodiscard]] constexpr bit_flag_set(xstd::from_blocks_t, Block value) noexcept
        {
                assert((bits::detail::mask_fits<Block>(value, N)));
                block() = value;
        }

        // The interop value is the block; unconstrained, since MSVC drops a constrained inherited converter.
        [[nodiscard]] constexpr explicit(false) bit_flag_set(interop_param value) noexcept // NOLINT(misc-explicit-constructor)
        {
                block() = bits_of(value);
        }

        [[nodiscard]] constexpr explicit(false) operator interop_param() const noexcept // NOLINT(misc-explicit-constructor)
                requires has_interop
        {
                return bits::detail::to_mask<Interop>(block());
        }

        // A swap on the base loses to any exact match on the flag type, so the flag type declares its own.
        friend constexpr auto swap(BitMask& x, BitMask& y) noexcept(noexcept(x.swap(y)))
                -> void
        {
                x.swap(y);
        }

        // Kept in view beside the all-of overload, which would hide the contains(Key) the base calls on BitMask.
        using base_type::contains;

        // All of other's flags, as bitflags' and Swift's contains: with one flag it agrees with contains(Key).
        [[nodiscard]] constexpr auto contains(BitMask const& other) const noexcept
                -> bool
        {
                return other.is_subset_of(*this);
        }

        // The key must name a position below the universe's size.
        [[nodiscard]] constexpr auto operator[](Key key) noexcept
                -> reference
        {
                return {&block(), position_of(key)};
        }

        [[nodiscard]] constexpr auto operator[](Key key) const noexcept
                -> bool
        {
                return this->contains(key);
        }

        // The forms over the base and over a key, beside the four over the interop mask below.
        using base_type::operator&=;
        using base_type::operator|=;
        using base_type::operator^=;
        using base_type::operator-=;

        // Total, as the binary & and - are: the value's bits at or above N meet nothing in this block.
        constexpr auto operator&=(interop_param other) noexcept
                -> BitMask&
                requires has_interop
        {
                block() = static_cast<Block>(block() & low_bits_of(other));
                return self();
        }

        constexpr auto operator-=(interop_param other) noexcept
                -> BitMask&
                requires has_interop
        {
                block() = static_cast<Block>(block() & static_cast<Block>(~low_bits_of(other)));
                return self();
        }

        // These two would bring the value's bits above N into the block, which they must not have.
        constexpr auto operator|=(interop_param other) noexcept
                -> BitMask&
                requires has_interop
        {
                block() = static_cast<Block>(block() | bits_of(other));
                return self();
        }

        constexpr auto operator^=(interop_param other) noexcept
                -> BitMask&
                requires has_interop
        {
                block() = static_cast<Block>(block() ^ bits_of(other));
                return self();
        }

        // Templates over the mask's exact type, so that a flag type converting to it is never deduced as one.
        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator==(BitMask const& lhs, Mask rhs) noexcept
                -> bool
                requires has_interop
        {
                return lhs.block() == low_bits_of(rhs) and bits::detail::mask_fits<Block>(rhs, N);
        }

        // The rest in both orders, exact where the mask's own needs a conversion; no bit at or above N.
        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator|(BitMask const& lhs, Mask rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return lhs | from_block(bits_of(rhs));
        }

        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator|(Mask lhs, BitMask const& rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return from_block(bits_of(lhs)) | rhs;
        }

        // Total: truncating the value to N bits is exact, since the flag set has no bit above N for it to keep.
        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator&(BitMask const& lhs, Mask rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return lhs & from_block(low_bits_of(rhs));
        }

        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator&(Mask lhs, BitMask const& rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return from_block(low_bits_of(lhs)) & rhs;
        }

        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator^(BitMask const& lhs, Mask rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return lhs ^ from_block(bits_of(rhs));
        }

        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator^(Mask lhs, BitMask const& rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return from_block(bits_of(lhs)) ^ rhs;
        }

        // Total, as & is; the mirror is not, the value's bits above N being what it would keep.
        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator-(BitMask const& lhs, Mask rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return lhs - from_block(low_bits_of(rhs));
        }

        template<std::same_as<interop_param> Mask>
        [[nodiscard]] friend constexpr auto operator-(Mask lhs, BitMask const& rhs) noexcept
                -> BitMask
                requires has_interop
        {
                return from_block(bits_of(lhs)) - rhs;
        }

private:
        [[nodiscard]] constexpr auto self() noexcept
                -> BitMask&
        {
                return static_cast<BitMask&>(*this);
        }

        // The one block, every position below N in it, named by a key or not.
        [[nodiscard]] constexpr auto block() noexcept
                -> Block&
        {
                return bits::detail::storage_access::bits(*this)[0];
        }

        [[nodiscard]] constexpr auto block() const noexcept
                -> Block
        {
                return bits::detail::storage_access::bits(*this)[0];
        }

        // A flag type holding the block, built through the default constructor so BitMask need inherit no other.
        [[nodiscard]] static constexpr auto from_block(Block value) noexcept
                -> BitMask
        {
                auto nrv                                = BitMask();
                static_cast<bit_flag_set&>(nrv).block() = value;
                return nrv;
        }

        // A key names one position below the universe's size.
        [[nodiscard]] static constexpr auto position_of(Key key) noexcept
                -> std::size_t
        {
                auto const pos = KeyMapping::to_index(key);
                assert(pos < num_keys());
                return pos;
        }

        // The value's bits below N, which are all of it that can meet a bit of this block.
        [[nodiscard]] static constexpr auto low_bits_of(interop_param value) noexcept
                -> Block
        {
                return bits::detail::low_mask_bits<Block>(value, N);
        }

        // Where a bit of the value would enter the result, it must have none at or above N.
        [[nodiscard]] static constexpr auto bits_of(interop_param value) noexcept
                -> Block
        {
                assert((bits::detail::mask_fits<Block>(value, N)));
                return low_bits_of(value);
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_SET_HPP
