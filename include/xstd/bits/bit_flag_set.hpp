//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_FLAG_SET_HPP
#define XSTD_BITS_BIT_FLAG_SET_HPP

#include <xstd/bits/bit_blocks.hpp>                // bit_blocks_extent_v
#include <xstd/bits/bit_key_traits.hpp>            // bit_key_traits
#include <xstd/bits/detail/intrin.hpp>             // countr_zero, popcount
#include <xstd/bits/detail/shift.hpp>              // shl, shr
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <cassert>                                 // assert
#include <cstddef>                                 // ptrdiff_t, size_t
#include <iterator>                                // forward_iterator_tag, input_iterator_tag
#include <type_traits>                             // conditional_t, is_enum_v, is_void_v, make_unsigned_t, underlying_type_t
#include <utility>                                 // to_underlying

// A base for flag types: a mask word whose named bits are the keys, spelled as the bitmask enumeration it replaces.
namespace xstd {

// Derived is the flag type, whose constants are its flags; Interop, unless void, is the enumeration it converts with.
template<class Derived, class Key, xstd::unsigned_integer Block, std::size_t N = bit_blocks_extent_v<Block>, class KeyTraits = bit_key_traits<Key>, class Interop = void>
class bit_flag_set
{
        static_assert(N <= bit_blocks_extent_v<Block>);
        static_assert(std::is_void_v<Interop> or std::is_enum_v<Interop>);

        // The keys are the universe the traits close, else every position of the word.
        [[nodiscard]] static consteval auto num_keys() noexcept
                -> std::size_t
        {
                if constexpr (requires { KeyTraits::size; }) {
                        return KeyTraits::size;
                } else {
                        return N;
                }
        }

        static_assert(num_keys() <= N);

        // The lowest n positions, the whole word included.
        [[nodiscard]] static consteval auto low_bits(std::size_t n) noexcept
                -> Block
        {
                return n == bit_blocks_extent_v<Block> ? static_cast<Block>(~Block{}) : static_cast<Block>(bits::detail::shl(Block{1}, n) - Block{1});
        }

        static constexpr Block width_mask = low_bits(N);
        static constexpr Block key_mask   = low_bits(num_keys());

        static constexpr bool has_interop = not std::is_void_v<Interop>;

        // What a conversion with no interop enumeration takes and gives: a type nothing else names.
        struct no_interop
        {};

        using interop_param = std::conditional_t<has_interop, Interop, no_interop>;

        // Every position below N, named by a key or not; a bit at or above N is never set.
        Block m_bits{};

public:
        using key_type        = Key;
        using value_type      = Key;
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;
        using block_type      = Block;
        using key_traits_type = KeyTraits;
        using interop_type    = Interop;

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

        // The named flags still to visit, lowest first: a copy of the word, so an iterator refers to nothing.
        class iterator
        {
                Block m_rest{};

                friend bit_flag_set;

                [[nodiscard]] constexpr explicit iterator(Block rest) noexcept
                        : m_rest(rest)
                {}

        public:
                using iterator_concept  = std::forward_iterator_tag;
                using iterator_category = std::input_iterator_tag;
                using value_type        = Key;
                using difference_type   = std::ptrdiff_t;
                using pointer           = void;
                using reference         = Key;

                [[nodiscard]] iterator() = default;

                [[nodiscard]] friend auto operator==(iterator const&, iterator const&) -> bool = default;

                // The key itself, so its own namespace is the one ADL searches for a formatter or a switch.
                [[nodiscard]] constexpr auto operator*() const noexcept
                        -> Key
                {
                        assert(m_rest != Block{});
                        return KeyTraits::from_index(bits::detail::countr_zero(m_rest));
                }

                constexpr auto operator++() noexcept
                        -> iterator&
                {
                        assert(m_rest != Block{});
                        m_rest = static_cast<Block>(m_rest & static_cast<Block>(m_rest - Block{1}));
                        return *this;
                }

                constexpr auto operator++(int) noexcept
                        -> iterator
                {
                        auto nrv = *this;
                        ++*this;
                        return nrv;
                }
        };

        using const_iterator = iterator;

        // The empty set, which is the zero value [bitmask.types] asks for.
        [[nodiscard]] bit_flag_set() = default;

        // One flag, explicit so that a key never meets a flag type through a conversion.
        [[nodiscard]] constexpr explicit bit_flag_set(key_type key) noexcept
                : m_bits(bits::detail::shl(Block{1}, position_of(key)))
        {}

        // The interop enumeration's value is the mask word: implicit both ways, with no bit at or above N coming in.
        [[nodiscard]] constexpr explicit(false) bit_flag_set(interop_param value) noexcept // NOLINT(misc-explicit-constructor)
                requires has_interop
                : m_bits(bits_of(value))
        {}

        [[nodiscard]] constexpr explicit(false) operator interop_param() const noexcept // NOLINT(misc-explicit-constructor)
                requires has_interop
        {
                return static_cast<Interop>(static_cast<std::underlying_type_t<Interop>>(m_bits));
        }

        // The word as it is, unnamed positions included, which is bitflags' from_bits_retain; no bit at or above N.
        [[nodiscard]] static constexpr auto from_bits(block_type bits) noexcept
                -> Derived
        {
                assert((bits & static_cast<Block>(~width_mask)) == Block{});
                auto nrv                               = Derived();
                static_cast<bit_flag_set&>(nrv).m_bits = bits;
                return nrv;
        }

        [[nodiscard]] constexpr auto bits() const noexcept
                -> block_type
        {
                return m_bits;
        }

        // The named flags, in rank order; a position no key names is in the word but not in the range.
        [[nodiscard]] constexpr auto begin() const noexcept
                -> iterator
        {
                return iterator(static_cast<Block>(m_bits & key_mask));
        }

        [[nodiscard]] constexpr auto end() const noexcept
                -> iterator
        {
                return iterator();
        }

        // How many named flags are set, which is how far begin() is from end().
        [[nodiscard]] constexpr auto size() const noexcept
                -> size_type
        {
                return bits::detail::popcount(static_cast<Block>(m_bits & key_mask));
        }

        // Total over key_type, as std::set's is: a key past the universe is in no set.
        [[nodiscard]] constexpr auto contains(key_type key) const noexcept
                -> bool
        {
                auto const pos = KeyTraits::to_index(key);
                return pos < num_keys() and (bits::detail::shr(m_bits, pos) & Block{1}) != Block{};
        }

        // All of other's flags, as bitflags' and Swift's contains: with one flag it agrees with the overload above.
        [[nodiscard]] constexpr auto contains(Derived const& other) const noexcept
                -> bool
        {
                return (other.bits() & static_cast<Block>(~m_bits)) == Block{};
        }

        // Any of other's flags, asked positively, which is what flags code nearly always asks.
        [[nodiscard]] constexpr auto intersects(Derived const& other) const noexcept
                -> bool
        {
                return (m_bits & other.bits()) != Block{};
        }

        [[nodiscard]] constexpr auto is_subset_of(Derived const& other) const noexcept
                -> bool
        {
                return other.contains(self());
        }

        // The key must name a position below the universe's size.
        [[nodiscard]] constexpr auto operator[](key_type key) noexcept
                -> reference
        {
                return {&m_bits, position_of(key)};
        }

        [[nodiscard]] constexpr auto operator[](key_type key) const noexcept
                -> bool
        {
                return contains(key);
        }

        constexpr auto operator|=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits = static_cast<Block>(m_bits | other.bits());
                return self();
        }

        constexpr auto operator&=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits = static_cast<Block>(m_bits & other.bits());
                return self();
        }

        constexpr auto operator^=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits = static_cast<Block>(m_bits ^ other.bits());
                return self();
        }

        constexpr auto operator-=(Derived const& other) noexcept
                -> Derived&
        {
                m_bits = static_cast<Block>(m_bits & static_cast<Block>(~other.bits()));
                return self();
        }

        // Total, as the binary & and - are: the value's bits at or above N meet nothing in this word.
        constexpr auto operator&=(interop_param other) noexcept
                -> Derived&
                requires has_interop
        {
                m_bits = static_cast<Block>(m_bits & low_bits_of(other));
                return self();
        }

        constexpr auto operator-=(interop_param other) noexcept
                -> Derived&
                requires has_interop
        {
                m_bits = static_cast<Block>(m_bits & static_cast<Block>(~low_bits_of(other)));
                return self();
        }

        // Hidden friends over Derived itself, so a conversion to the interop enumeration never ties with one.
        [[nodiscard]] friend constexpr auto operator==(Derived const& lhs, Derived const& rhs) noexcept
                -> bool
        {
                return lhs.bits() == rhs.bits();
        }

        // Within N: the complement of none is every position of the word, named or not.
        [[nodiscard]] friend constexpr auto operator~(Derived const& x) noexcept
                -> Derived
        {
                return from_bits(static_cast<Block>(static_cast<Block>(~x.bits()) & width_mask));
        }

        [[nodiscard]] friend constexpr auto operator|(Derived const& lhs, Derived const& rhs) noexcept
                -> Derived
        {
                auto nrv = lhs;
                nrv |= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator&(Derived const& lhs, Derived const& rhs) noexcept
                -> Derived
        {
                auto nrv = lhs;
                nrv &= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator^(Derived const& lhs, Derived const& rhs) noexcept
                -> Derived
        {
                auto nrv = lhs;
                nrv ^= rhs;
                return nrv;
        }

        [[nodiscard]] friend constexpr auto operator-(Derived const& lhs, Derived const& rhs) noexcept
                -> Derived
        {
                auto nrv = lhs;
                nrv -= rhs;
                return nrv;
        }

        // Total against the interop enumeration: a value with a bit at or above N equals no flag set.
        [[nodiscard]] friend constexpr auto operator==(Derived const& lhs, interop_param rhs) noexcept
                -> bool
                requires has_interop
        {
                auto const bits = low_bits_of(rhs);
                return lhs.bits() == bits and static_cast<decltype(word_of(rhs))>(bits) == word_of(rhs);
        }

        // The rest in both orders, exact where the enumeration's own needs a conversion; no bit at or above N.
        [[nodiscard]] friend constexpr auto operator|(Derived const& lhs, interop_param rhs) noexcept
                -> Derived
                requires has_interop
        {
                return lhs | from_bits(bits_of(rhs));
        }

        [[nodiscard]] friend constexpr auto operator|(interop_param lhs, Derived const& rhs) noexcept
                -> Derived
                requires has_interop
        {
                return from_bits(bits_of(lhs)) | rhs;
        }

        // Total: truncating the value to N bits is exact, since the flag set has no bit above N for it to keep.
        [[nodiscard]] friend constexpr auto operator&(Derived const& lhs, interop_param rhs) noexcept
                -> Derived
                requires has_interop
        {
                return lhs & from_bits(low_bits_of(rhs));
        }

        [[nodiscard]] friend constexpr auto operator&(interop_param lhs, Derived const& rhs) noexcept
                -> Derived
                requires has_interop
        {
                return from_bits(low_bits_of(lhs)) & rhs;
        }

        [[nodiscard]] friend constexpr auto operator^(Derived const& lhs, interop_param rhs) noexcept
                -> Derived
                requires has_interop
        {
                return lhs ^ from_bits(bits_of(rhs));
        }

        [[nodiscard]] friend constexpr auto operator^(interop_param lhs, Derived const& rhs) noexcept
                -> Derived
                requires has_interop
        {
                return from_bits(bits_of(lhs)) ^ rhs;
        }

        // Total, as & is; the mirror is not, the value's bits above N being what it would keep.
        [[nodiscard]] friend constexpr auto operator-(Derived const& lhs, interop_param rhs) noexcept
                -> Derived
                requires has_interop
        {
                return lhs - from_bits(low_bits_of(rhs));
        }

        [[nodiscard]] friend constexpr auto operator-(interop_param lhs, Derived const& rhs) noexcept
                -> Derived
                requires has_interop
        {
                return from_bits(bits_of(lhs)) - rhs;
        }

private:
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

        // A key names one position below the universe's size.
        [[nodiscard]] static constexpr auto position_of(key_type key) noexcept
                -> std::size_t
        {
                auto const pos = KeyTraits::to_index(key);
                assert(pos < num_keys());
                return pos;
        }

        // The enumeration's value read in its unsigned counterpart.
        [[nodiscard]] static constexpr auto word_of(interop_param value) noexcept
        {
                return static_cast<std::make_unsigned_t<std::underlying_type_t<Interop>>>(std::to_underlying(value));
        }

        // The value's bits below N, which are all of it that can meet a bit of this word.
        [[nodiscard]] static constexpr auto low_bits_of(interop_param value) noexcept
                -> Block
        {
                return static_cast<Block>(static_cast<Block>(word_of(value)) & width_mask);
        }

        // Where a bit of the value would enter the result, it must have none at or above N.
        [[nodiscard]] static constexpr auto bits_of(interop_param value) noexcept
                -> Block
        {
                auto const bits = low_bits_of(value);
                assert(static_cast<decltype(word_of(value))>(bits) == word_of(value));
                return bits;
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_FLAG_SET_HPP
