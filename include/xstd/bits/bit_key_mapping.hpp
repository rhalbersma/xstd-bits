//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_KEY_MAPPING_HPP
#define XSTD_BITS_BIT_KEY_MAPPING_HPP

#include <xstd/bits/detail/ordinal.hpp>            // are_consecutive, key_distance, ordinal, ordinal_key, ordinal_t
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/type_traits/make_unsigned.hpp> // make_unsigned_t
#include <algorithm>                               // ranges::adjacent_find, ranges::lower_bound
#include <concepts>                                // same_as
#include <cstddef>                                 // size_t
#include <functional>                              // ranges::greater_equal, ranges::less
#include <iterator>                                // ranges::distance
#include <ranges>                                  // begin, end, range_value_t, size
#include <type_traits>                             // conditional_t, is_enum_v

// How a set owner's key maps onto a position and back: an order-preserving bijection onto the positions [0, N).
namespace xstd {

// Specialized beside an enumeration: values, a static constexpr std::array of its enumerators, ascending, each once.
template<class Enum>
struct enum_traits
{};

// The N consecutive keys from First, a contiguous range at positions 0 through N - 1, its universe closed at N.
template<class Key, Key First, std::size_t N>
        requires bits::detail::ordinal_key<Key>
struct bit_range_mapping
{
        static constexpr std::size_t size = N;

        // A key outside the range ranks at size or above, a key below First wrapping round to the top.
        [[nodiscard]] static constexpr auto to_index(Key key) noexcept
                -> std::size_t
        {
                return bits::detail::key_distance(First, key);
        }

        // Modular in the unsigned counterpart, so the most negative First still reaches every key of the range.
        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                using unsigned_type = xstd::make_unsigned_t<bits::detail::ordinal_t<Key>>;
                return static_cast<Key>(static_cast<unsigned_type>(static_cast<unsigned_type>(bits::detail::ordinal(First)) + static_cast<unsigned_type>(index)));
        }
};

// The keys listed in Keys, each at its rank there, so a gap between two keys costs no position.
template<class Key, auto const& Keys>
        requires bits::detail::ordinal_key<Key> and std::same_as<std::ranges::range_value_t<decltype(Keys)>, Key>
struct bit_find_mapping
{
private:
        static constexpr auto ordinal = [](Key key) noexcept -> bits::detail::ordinal_t<Key> { return bits::detail::ordinal(key); };

        // Strictly ascending, which an order-preserving rank needs and which also rules out a key listed twice.
        static_assert(std::ranges::adjacent_find(Keys, std::ranges::greater_equal{}, ordinal) == std::ranges::end(Keys));

public:
        static constexpr std::size_t size = std::ranges::size(Keys);

        // A binary search for the key's rank; a key not in Keys ranks at size.
        [[nodiscard]] static constexpr auto to_index(Key key) noexcept
                -> std::size_t
        {
                auto const first = std::ranges::lower_bound(Keys, ordinal(key), std::ranges::less{}, ordinal);
                if (first == std::ranges::end(Keys) or ordinal(*first) != ordinal(key)) {
                        return size;
                }
                return static_cast<std::size_t>(std::ranges::distance(std::ranges::begin(Keys), first));
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                return std::ranges::begin(Keys)[index];
        }
};

// to_index preserves order, a < b exactly where to_index(a) < to_index(b), and from_index inverts it.
template<class Key>
struct bit_key_mapping;

// The identity, with no size: an unsigned key is its own position, in a universe left open.
template<xstd::unsigned_integer Key>
struct bit_key_mapping<Key>
{
        // A key wider than std::size_t must name a position: key <= std::numeric_limits<std::size_t>::max().
        [[nodiscard]] static constexpr auto to_index(Key key) noexcept
                -> std::size_t
        {
                return static_cast<std::size_t>(key);
        }

        // Only a position some key named comes back, so a narrower Key holds it.
        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> Key
        {
                return static_cast<Key>(index);
        }
};

// An enumeration whose author listed its values: a range where they run without a gap, else a search of the list.
template<class Key>
        requires std::is_enum_v<Key> and requires { enum_traits<Key>::values; }
struct bit_key_mapping<Key> : std::conditional_t<bits::detail::are_consecutive(enum_traits<Key>::values), bit_range_mapping<Key, *std::ranges::begin(enum_traits<Key>::values), std::ranges::size(enum_traits<Key>::values)>, bit_find_mapping<Key, enum_traits<Key>::values>>
{};

} // namespace xstd

#endif // XSTD_BITS_BIT_KEY_MAPPING_HPP
