//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef XSTD_BITS_BIT_CONCEPTS_BIT_CONSTRUCTIBLE_FROM_HPP
#define XSTD_BITS_BIT_CONCEPTS_BIT_CONSTRUCTIBLE_FROM_HPP

#include <xstd/bits/detail/owned_bit_blocks.hpp> // owned_bit_blocks
#include <xstd/bits/from_blocks.hpp>             // from_blocks_t
#include <concepts>                              // constructible_from
#include <type_traits>                           // remove_cvref_t

namespace xstd {

// Blocks that are bit storage for To as they are: the tag constructor takes them, and nothing is copied or shifted.
template<class To, class Blocks>
concept bit_constructible_from =
        bits::detail::owned_bit_blocks<std::remove_cvref_t<Blocks>> and
        std::constructible_from<To, from_blocks_t, Blocks>;

} // namespace xstd

#endif // XSTD_BITS_BIT_CONCEPTS_BIT_CONSTRUCTIBLE_FROM_HPP
