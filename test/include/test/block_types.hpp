//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_BLOCK_TYPES_HPP
#define TEST_BLOCK_TYPES_HPP

#include <test/ext_int128.hpp>                     // TEST_HAS_ABSL_INT128, TEST_HAS_BOOST_INT128, uint128
#include <test/uint128.hpp>                        // TEST_HAS_UINT128, uint128
#include <xstd/ints/bit.hpp>                       // countl_zero, countr_zero, popcount
#include <xstd/ints/concepts/unsigned_integer.hpp> // unsigned_integer
#include <xstd/ints/limits.hpp>                    // numeric_limits
#include <cstddef>                                 // size_t
#include <cstdint>                                 // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                   // tuple, tuple_cat
#include <utility>                                 // declval

// The Block models and the extents worth instantiating, assembled once: all three containers take <class Block, size_t N>.
namespace test {

// What the containers actually require of a Block: the concept they constrain on, and the three functions
// detail/intrin.hpp calls. std::unsigned_integral is NOT this question and must not stand in for it -- it is
// closed, so every 128-bit integer CLASS fails it while serving as a Block perfectly well.
//
// This must be spelled HERE, below every adapter include above, and not in a header of its own. The calls are
// qualified, so their candidates are the overloads visible at THIS point; an adapter included afterwards
// declares its overload too late to be one, and the concept would then answer false for a type that works. A
// separate header could not be relied on to land below them, the include lists here being sorted by name.
template<class Block>
concept block_basis =
        xstd::unsigned_integer<Block> and
        requires (Block x) {
                xstd::countl_zero(x);
                xstd::countr_zero(x);
                xstd::popcount(x);
        };

// Each flag held to the basis it claims. An implication, NOT an equality: where a flag is on, the basis is
// really there. The equality this replaces asked std::unsigned_integral of xstd::uint128, which is the wrong
// question in both directions -- false for every integer class that works as a Block, so it would deny the
// MSVC half outright, and true in dialects where <bit> still declines the type. The converse is not worth
// asserting either: a basis a flag declines to use costs coverage, not correctness. [design.md#uint128-support]
static_assert(not has_uint128 or block_basis<xstd::uint128>);
#ifdef TEST_HAS_ABSL_INT128
static_assert(block_basis<absl::uint128>);
#endif
#ifdef TEST_HAS_BOOST_INT128
static_assert(block_basis<boost::int128::uint128>);
#endif

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

// The widest Blocks, which cross a boundary for two reasons the narrow ones cannot cover.
//
// First, promotion. uint8_t and uint16_t PROMOTE: every block operation on them has an int intermediate, and
// the code masks back from it. uint32_t upward do not promote at all, so the block's own width is the whole
// modulus and there is nothing to mask back from. Straddling at narrow words alone therefore exercises only
// the promoting path, and "the arithmetic follows digits, not the carrier" holds within a block but is
// untested across one.
//
// Second, and only since the blocks reach xstd's bit basis rather than <bit> directly: a Block need not be a
// scalar at all. All three below are 128 bits wide and none of them promotes, but xstd::uint128 is the
// compiler's own extension on every target except an MSVC-ABI one, where it is std::_Unsigned128 -- and these
// two are CLASSES outright, whose operators are ordinary functions returning class type. That is what catches
// a container quietly assuming a Block is a scalar, as detail/pred.hpp's intersects did: it returned lhs & rhs
// into a bool, which a builtin converts to implicitly and an integer class, whose operator bool is explicit,
// does not. [design.md#uint128-support]
//
// One optional tuple per candidate, concatenated: a type that is not there contributes an empty tuple, so the
// list composes without any comma bookkeeping between the #ifs.
using wide_word_types = decltype(std::tuple_cat(
        std::declval<std::tuple<
#ifdef TEST_HAS_UINT128
                xstd::uint128
#endif
        >>(),
        std::declval<std::tuple<
#ifdef TEST_HAS_ABSL_INT128
                absl::uint128
#endif
        >>(),
        std::declval<std::tuple<
#ifdef TEST_HAS_BOOST_INT128
                boost::int128::uint128
#endif
        >>()
));

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
