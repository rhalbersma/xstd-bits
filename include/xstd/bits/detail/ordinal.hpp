//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_ORDINAL_HPP
#define XSTD_BITS_DETAIL_ORDINAL_HPP

#include <xstd/ints/concepts/integer.hpp>          // integer
#include <xstd/ints/type_traits/make_unsigned.hpp> // make_unsigned_t
#include <algorithm>                               // ranges::all_of
#include <cstddef>                                 // size_t
#include <ranges>                                  // begin, iota, range_difference_t, range_value_t, size
#include <type_traits>                             // is_enum_v
#include <utility>                                 // to_underlying

// A key as the integer a mapping orders and counts it by: an enumeration's underlying value, an integer as it is.
namespace xstd::bits::detail {

template<class Key>
concept ordinal_key = xstd::integer<Key> or std::is_enum_v<Key>;

template<ordinal_key Key>
[[nodiscard]] constexpr auto ordinal(Key key) noexcept
{
        if constexpr (std::is_enum_v<Key>) {
                return std::to_underlying(key);
        } else {
                return key;
        }
}

template<ordinal_key Key>
using ordinal_t = decltype(detail::ordinal(Key()));

// A key's ordinal made unsigned, where key arithmetic wraps rather than overflows and where a key's bits are a block.
template<ordinal_key Key>
using unsigned_ordinal_t = xstd::make_unsigned_t<ordinal_t<Key>>;

// Unsigned, so the distance from the most negative key is exact rather than an overflow.
template<ordinal_key Key>
[[nodiscard]] constexpr auto key_distance(Key from, Key to) noexcept
        -> std::size_t
{
        using unsigned_type = unsigned_ordinal_t<Key>;
        return static_cast<std::size_t>(static_cast<unsigned_type>(static_cast<unsigned_type>(detail::ordinal(to)) - static_cast<unsigned_type>(detail::ordinal(from))));
}

// Each key one above the one before, so a key's rank is its distance from the first.
template<class Keys>
[[nodiscard]] consteval auto are_consecutive(Keys const& keys) noexcept
        -> bool
{
        return std::ranges::all_of(std::views::iota(0UZ, std::ranges::size(keys)), [&](std::size_t i) noexcept -> bool { return detail::key_distance(*std::ranges::begin(keys), std::ranges::begin(keys)[static_cast<std::ranges::range_difference_t<Keys const&>>(i)]) == i; });
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_ORDINAL_HPP
