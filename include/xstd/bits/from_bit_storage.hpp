//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_FROM_BIT_STORAGE_HPP
#define XSTD_BITS_FROM_BIT_STORAGE_HPP

#include <xstd/bits/bit_storage.hpp> // owned_bit_storage
#include <concepts>                  // constructible_from
#include <type_traits>               // remove_cvref_t

// The tag that says an argument's blocks are read as bits, as std::from_range says a range's elements are read.
namespace xstd {

struct from_bit_storage_t
{
        explicit from_bit_storage_t() = default;
};

inline constexpr auto from_bit_storage = from_bit_storage_t();

// Blocks that are bit storage for To as they are: the tag constructor takes them, and nothing is copied or shifted.
template<class To, class Blocks>
concept bit_constructible_from =
        xstd::owned_bit_storage<std::remove_cvref_t<Blocks>> and
        std::constructible_from<To, from_bit_storage_t, Blocks>;

} // namespace xstd

#endif // XSTD_BITS_FROM_BIT_STORAGE_HPP
