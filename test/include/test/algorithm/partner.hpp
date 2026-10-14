//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_ALGORITHM_PARTNER_HPP
#define TEST_ALGORITHM_PARTNER_HPP

#include <xstd/bits/bit_type_traits/bit_rebind.hpp> // bit_rebind
#include <concepts>                                 // same_as
#include <cstdint>                                  // uint64_t, uint8_t
#include <ranges>                                   // range_value_t
#include <type_traits>                              // conditional_t, type_identity

// Another container holding the same value over other blocks, for the algorithms that compare across block types.
namespace test::algorithm {

template<class X>
using block_of_t = std::ranges::range_value_t<typename X::block_container_type>;

// The narrowest Block where X's is any other, and the widest builtin where X's is the narrowest.
template<class X>
using other_block_t = std::conditional_t<std::same_as<block_of_t<X>, std::uint8_t>, std::uint64_t, std::uint8_t>;

template<class X>
struct partner
{
        using type = X;
};

template<class X>
        requires requires { typename xstd::bit_rebind<other_block_t<X>, X>; }
struct partner<X>
{
        using type = xstd::bit_rebind<other_block_t<X>, X>;
};

// X over the other Block, or X itself where its storage cannot be rebound.
template<class X>
using partner_t = partner<X>::type;

} // namespace test::algorithm

#endif // TEST_ALGORITHM_PARTNER_HPP
