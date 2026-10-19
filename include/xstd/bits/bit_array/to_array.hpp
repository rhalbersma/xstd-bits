//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_ARRAY_TO_ARRAY_HPP
#define XSTD_BITS_BIT_ARRAY_TO_ARRAY_HPP

#include <xstd/bits/bit_array/bit_array.hpp> // bit_array
#include <concepts>                          // same_as
#include <cstddef>                           // size_t
#include <type_traits>                       // remove_cv_t
#include <utility>                           // move

namespace xstd {

// [array.creation]'s to_array, of bits: a built-in array names no block type, so the default one is taken.
template<class T, std::size_t N>
[[nodiscard]] constexpr auto to_bit_array(T (&a)[N]) // NOLINT(modernize-avoid-c-arrays): a built-in array is what it converts.
        -> bit_array<N>
{
        static_assert(std::same_as<std::remove_cv_t<T>, bool>, "[array.creation]/1: the element is a bit");
        return bit_array<N>(a);
}

// [array.creation]'s second overload, moving the elements as to_array does.
template<class T, std::size_t N>
[[nodiscard]] constexpr auto to_bit_array(T (&&a)[N]) // NOLINT(modernize-avoid-c-arrays): a built-in array is what it converts.
        -> bit_array<N>
{
        static_assert(std::same_as<std::remove_cv_t<T>, bool>, "[array.creation]/4: the element is a bit");
        return bit_array<N>(std::move(a));
}

} // namespace xstd

#endif // XSTD_BITS_BIT_ARRAY_TO_ARRAY_HPP
