//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BLOCK_TYPES_HPP
#define TEST_BLOCK_TYPES_HPP

#include <test/uint128.hpp>     // TEST_HAS_UINT128, uint128
#include <xstd/ints/limits.hpp> // numeric_limits
#include <cstddef>              // size_t
#include <cstdint>              // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                // tuple, tuple_cat
#include <utility>              // declval

// The Block models and the extents worth instantiating, assembled once: all three containers take <class Block, size_t N>.
namespace test {

template<class Block>
inline constexpr auto digits_v = static_cast<std::size_t>(xstd::numeric_limits<Block>::digits);

// Every Block the library is instantiated over; xstd::uint128 rides on <bit>, so it comes and goes with test/uint128.hpp.
using word_types = std::tuple
<       std::uint8_t
,       std::uint16_t
,       std::uint32_t
,       std::uint64_t
#ifdef TEST_HAS_UINT128
,       xstd::uint128
#endif
>;

// The Blocks narrow enough to straddle block boundaries exhaustively; a bit-precise unsigned _BitInt(4) belongs here.
using narrow_word_types = std::tuple
<       std::uint8_t
>;

// The widest Block, which crosses a boundary for a reason the narrow ones cannot cover. uint8_t and uint16_t
// PROMOTE: every block operation on them has an int intermediate, and the code masks back from it. uint32_t
// upward do not promote at all, so the block's own width is the whole modulus and there is nothing to mask
// back from. Straddling at narrow words alone therefore exercises only the promoting path, and "the arithmetic
// follows digits, not the carrier" holds within a block but is untested across one. xstd::uint128 is the
// extreme of the non-promoting half and the only Block whose carrier is not a standard unsigned integer type,
// so it is the one worth the extents. [design.md#uint128-support]
using wide_word_types = std::tuple
<
#ifdef TEST_HAS_UINT128
        xstd::uint128
#endif
>;

// One block's worth of extents: empty, a single bit, and exactly one full block -- the same cost at any width.
template<template<class, std::size_t> class C, class Block>
using in_block_extents = std::tuple
<       C<Block, 0>
,       C<Block, 1>
,       C<Block, digits_v<Block>>
>;

// The extents that straddle a block boundary, at the narrowest word only: the arithmetic follows digits, not the carrier.
template<template<class, std::size_t> class C, class Block>
using straddling_extents = std::tuple
<       C<Block,     digits_v<Block> - 1>
,       C<Block,     digits_v<Block> + 1>
,       C<Block, (2 * digits_v<Block>) - 1>
,       C<Block, 2 * digits_v<Block>      >
,       C<Block, (2 * digits_v<Block>) + 1>
,       C<Block, 3 * digits_v<Block>      >
>;

namespace detail {

template<template<class, std::size_t> class C, template<template<class, std::size_t> class, class> class Extents, class... Blocks>
auto expand(std::tuple<Blocks...>) -> decltype(std::tuple_cat(std::declval<Extents<C, Blocks>>()...));

} // namespace detail

// Every word type at the extents it can afford: all within one block, and the narrow ones across boundaries too.
template<template<class, std::size_t> class C>
using graded_extents = decltype(std::tuple_cat(
        std::declval<decltype(detail::expand<C, in_block_extents>(std::declval<word_types>()))>(),
        std::declval<decltype(detail::expand<C, straddling_extents>(std::declval<narrow_word_types>()))>()
));

// The widest Block across block boundaries, for the suites that can afford it: every case they run per type is
// a static_assert or a single pass over the positions, so three blocks of 128 costs what one block costs.
// Deliberately NOT folded into graded_extents, which feeds suites whose per-type work is superlinear.
template<template<class, std::size_t> class C>
using wide_extents = decltype(detail::expand<C, straddling_extents>(std::declval<wide_word_types>()));

} // namespace test

#endif // TEST_BLOCK_TYPES_HPP
