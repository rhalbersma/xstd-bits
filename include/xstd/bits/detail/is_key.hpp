//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_DETAIL_IS_KEY_HPP
#define XSTD_BITS_DETAIL_IS_KEY_HPP

#include <concepts> // same_as

// Whether a value is a key of a mapping's universe, asked of any mapping, with is_key or without.
namespace xstd::bits::detail {

// The mapping's own answer where it gives one; a mapping without is_key leaves its universe open to every key.
template<class Mapping, class Key>
[[nodiscard]] constexpr auto is_key(Key const& key) noexcept
        -> bool
{
        if constexpr (requires { { Mapping::is_key(key) } -> std::same_as<bool>; }) {
                return Mapping::is_key(key);
        } else {
                return true;
        }
}

} // namespace xstd::bits::detail

#endif // XSTD_BITS_DETAIL_IS_KEY_HPP
