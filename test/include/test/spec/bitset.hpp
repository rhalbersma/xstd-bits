//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_BITSET_HPP
#define TEST_SPEC_BITSET_HPP

#include <test/uint128.hpp>                     // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                 // basic_bitset
#include <xstd/bits/bounded_bitset.hpp>         // basic_bounded_bitset
#include <xstd/bits/dynamic_bitset.hpp>         // basic_dynamic_bitset
#include <xstd/bits/ext/boost/small_bitset.hpp> // basic_small_bitset
#include <boost/dynamic_bitset.hpp>             // dynamic_bitset
#include <bitset>                               // bitset
#include <cstdint>                              // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                // tuple, tuple_cat
#include <utility>                              // declval

// The candidates for the bitset reading, the models first, graded by what a sweep can afford.
namespace test::spec::bitset {

// A static width is std::bitset's and a run-time width boost::dynamic_bitset's.
template<class... StaticWidths>
using models = std::tuple<StaticWidths..., boost::dynamic_bitset<>>;

using dynamic = std::tuple<xstd::basic_dynamic_bitset<std::uint8_t>, xstd::basic_dynamic_bitset<std::uint64_t>>;

// Every Block at an empty width, a single bit, and either side of its first two block boundaries.
using fixed_every_width = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 1>, xstd::basic_bitset<std::uint8_t, 7>, xstd::basic_bitset<std::uint8_t, 8>, xstd::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 15>, xstd::basic_bitset<std::uint8_t, 16>, xstd::basic_bitset<std::uint8_t, 17>, xstd::basic_bitset<std::uint8_t, 24>, xstd::basic_bitset<std::uint16_t, 0>, xstd::basic_bitset<std::uint16_t, 1>, xstd::basic_bitset<std::uint16_t, 15>, xstd::basic_bitset<std::uint16_t, 16>, xstd::basic_bitset<std::uint16_t, 17>, xstd::basic_bitset<std::uint16_t, 31>, xstd::basic_bitset<std::uint16_t, 32>, xstd::basic_bitset<std::uint16_t, 33>, xstd::basic_bitset<std::uint16_t, 48>, xstd::basic_bitset<std::uint32_t, 0>, xstd::basic_bitset<std::uint32_t, 1>, xstd::basic_bitset<std::uint32_t, 31>, xstd::basic_bitset<std::uint32_t, 32>, xstd::basic_bitset<std::uint32_t, 33>, xstd::basic_bitset<std::uint32_t, 63>, xstd::basic_bitset<std::uint32_t, 64>, xstd::basic_bitset<std::uint32_t, 65>, xstd::basic_bitset<std::uint64_t, 0>, xstd::basic_bitset<std::uint64_t, 1>, xstd::basic_bitset<std::uint64_t, 63>, xstd::basic_bitset<std::uint64_t, 64>, xstd::basic_bitset<std::uint64_t, 65>
#ifdef TEST_HAS_UINT128

                                     ,
                                     xstd::basic_bitset<xstd::uint128, 0>, xstd::basic_bitset<xstd::uint128, 1>, xstd::basic_bitset<xstd::uint128, 127>, xstd::basic_bitset<xstd::uint128, 128>, xstd::basic_bitset<xstd::uint128, 129>

#endif
                                     >;

using bounded_every_width = std::tuple<xstd::basic_bounded_bitset<std::uint8_t, 0>, xstd::basic_bounded_bitset<std::uint8_t, 9>, xstd::basic_bounded_bitset<std::uint8_t, 17>, xstd::basic_bounded_bitset<std::uint64_t, 65>>;

using small_every_width = std::tuple<xstd::basic_small_bitset<std::uint8_t, 9>, xstd::basic_small_bitset<std::uint64_t, 64>>;

// The narrowest Block across two boundaries, and every other Block at one width that spans three narrow blocks.
using fixed_boundary_widths = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 1>, xstd::basic_bitset<std::uint8_t, 8>, xstd::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 16>, xstd::basic_bitset<std::uint8_t, 17>, xstd::basic_bitset<std::uint8_t, 24>, xstd::basic_bitset<std::uint16_t, 24>, xstd::basic_bitset<std::uint32_t, 24>, xstd::basic_bitset<std::uint64_t, 24>
#ifdef TEST_HAS_UINT128

                                         ,
                                         xstd::basic_bitset<xstd::uint128, 24>

#endif
                                         >;

using bounded_boundary_widths = std::tuple<xstd::basic_bounded_bitset<std::uint8_t, 0>, xstd::basic_bounded_bitset<std::uint8_t, 9>, xstd::basic_bounded_bitset<std::uint8_t, 17>, xstd::basic_bounded_bitset<std::uint8_t, 24>, xstd::basic_bounded_bitset<std::uint64_t, 24>>;

// The inline blocks hold the first 16 positions, so a sweep to its limit spills onto the heap.
using small_boundary_widths = std::tuple<xstd::basic_small_bitset<std::uint8_t, 9>>;

// Either side of the first byte boundary, and every Block at the byte.
using fixed_byte_widths = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 8>, xstd::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 17>, xstd::basic_bitset<std::uint16_t, 8>, xstd::basic_bitset<std::uint32_t, 8>, xstd::basic_bitset<std::uint64_t, 8>
#ifdef TEST_HAS_UINT128

                                     ,
                                     xstd::basic_bitset<xstd::uint128, 8>

#endif
                                     >;

// Either side of the first byte boundary, and every Block at one width past it.
using fixed_few_widths = std::tuple<xstd::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 8>, xstd::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 17>, xstd::basic_bitset<std::uint16_t, 17>, xstd::basic_bitset<std::uint32_t, 17>, xstd::basic_bitset<std::uint64_t, 17>
#ifdef TEST_HAS_UINT128

                                    ,
                                    xstd::basic_bitset<xstd::uint128, 17>

#endif
                                    >;

using bounded_few_widths = std::tuple<xstd::basic_bounded_bitset<std::uint8_t, 9>, xstd::basic_bounded_bitset<std::uint8_t, 17>>;

using small_few_widths = std::tuple<xstd::basic_small_bitset<std::uint8_t, 9>>;

// Widths no exhaustive sweep reaches, each off a block boundary of its own Block or of the byte.
using fixed_random_widths = std::tuple<xstd::basic_bitset<std::uint8_t, 257>, xstd::basic_bitset<std::uint32_t, 1023>, xstd::basic_bitset<std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                                       ,
                                       xstd::basic_bitset<xstd::uint128, 2049>

#endif
                                       >;

using bounded_random_widths = std::tuple<xstd::basic_bounded_bitset<std::uint64_t, 4097>>;

// The inline blocks hold half of what a sample spans, so the dense samples spill onto the heap.
using small_random_widths = std::tuple<xstd::basic_small_bitset<std::uint64_t, 1024>>;

// The widest grid, for the sweeps that are constant per type.
using every_width = decltype(std::tuple_cat(std::declval<models<std::bitset<0>, std::bitset<1>, std::bitset<31>, std::bitset<32>, std::bitset<33>, std::bitset<63>, std::bitset<64>, std::bitset<65>>>(), std::declval<fixed_every_width>(), std::declval<dynamic>(), std::declval<bounded_every_width>(), std::declval<small_every_width>()));

// For the sweeps linear in the width.
using boundary_widths = decltype(std::tuple_cat(std::declval<models<std::bitset<0>, std::bitset<1>, std::bitset<64>>>(), std::declval<fixed_boundary_widths>(), std::declval<dynamic>(), std::declval<bounded_boundary_widths>(), std::declval<small_boundary_widths>()));

// For the sweeps quadratic in the width.
using byte_widths = decltype(std::tuple_cat(std::declval<models<std::bitset<0>, std::bitset<8>>>(), std::declval<fixed_byte_widths>(), std::declval<dynamic>(), std::declval<bounded_few_widths>(), std::declval<small_few_widths>()));

// For the sweeps cubic or quartic in the width.
using few_widths = decltype(std::tuple_cat(std::declval<models<std::bitset<0>, std::bitset<17>>>(), std::declval<fixed_few_widths>(), std::declval<dynamic>(), std::declval<bounded_few_widths>(), std::declval<small_few_widths>()));

// For the sampled sweeps, whose cost is the sample count rather than the width.
using random_widths = decltype(std::tuple_cat(std::declval<models<std::bitset<1025>>>(), std::declval<fixed_random_widths>(), std::declval<dynamic>(), std::declval<bounded_random_widths>(), std::declval<small_random_widths>()));

} // namespace test::spec::bitset

#endif // TEST_SPEC_BITSET_HPP
