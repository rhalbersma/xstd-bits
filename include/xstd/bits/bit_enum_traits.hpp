//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ENUM_TRAITS_HPP
#define XSTD_BITS_BIT_ENUM_TRAITS_HPP

#include <algorithm>   // adjacent_find, all_of, find
#include <cstddef>     // size_t
#include <functional>  // ranges::greater_equal
#include <iterator>    // ranges::distance
#include <ranges>      // begin, end, iota, size
#include <type_traits> // is_enum_v, make_unsigned_t, underlying_type_t
#include <utility>     // to_underlying

// An enumeration described by its author, and that description placed in bits, as std::char_traits places a character.
namespace xstd {

// Specialized beside an enumeration E: a static constexpr std::array values of its enumerators, ascending, each once.
template<class E>
struct enum_traits
{};

// Left incomplete for a type listing no values, so naming a member of it fails in the immediate context.
template<class E>
struct bit_enum_traits;

// The enumerators' ranks in values are their positions, so a gap between two values costs no bit.
template<class E>
        requires std::is_enum_v<E> and requires { enum_traits<E>::values; }
struct bit_enum_traits<E>
{
private:
        static constexpr auto const& values = enum_traits<E>::values;

        // The arithmetic is unsigned, so the distance from a negative first enumerator is exact rather than an overflow.
        using unsigned_type = std::make_unsigned_t<std::underlying_type_t<E>>;

        [[nodiscard]] static constexpr auto offset(E from, E to) noexcept
                -> std::size_t
        {
                return static_cast<std::size_t>(static_cast<unsigned_type>(static_cast<unsigned_type>(std::to_underlying(to)) - static_cast<unsigned_type>(std::to_underlying(from))));
        }

        // Strictly ascending, which an order-preserving rank needs and which also rules out a value listed twice.
        static_assert(std::ranges::adjacent_find(values, std::ranges::greater_equal{}, [](E e) noexcept -> std::underlying_type_t<E> { return std::to_underlying(e); }) == std::ranges::end(values));

        // Each value one above the last, so a rank is a subtraction rather than a search.
        [[nodiscard]] static consteval auto is_contiguous() noexcept
                -> bool
        {
                return std::ranges::all_of(std::views::iota(0UZ, std::ranges::size(values)), [](std::size_t i) noexcept -> bool { return offset(values[0], values[i]) == i; });
        }

public:
        static constexpr std::size_t size = std::ranges::size(values);

        // The key must be one of the listed values; one that is not ranks at size or above.
        [[nodiscard]] static constexpr auto to_index(E key) noexcept
                -> std::size_t
        {
                if constexpr (is_contiguous()) {
                        return offset(values[0], key);
                } else {
                        return static_cast<std::size_t>(std::ranges::distance(std::ranges::begin(values), std::ranges::find(values, key)));
                }
        }

        [[nodiscard]] static constexpr auto from_index(std::size_t index) noexcept
                -> E
        {
                return values[index];
        }
};

} // namespace xstd

#endif // XSTD_BITS_BIT_ENUM_TRAITS_HPP
