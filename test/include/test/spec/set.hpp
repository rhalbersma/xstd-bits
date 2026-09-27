//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_SET_HPP
#define TEST_SPEC_SET_HPP

#include <test/flat_set.hpp>                             // IWYU pragma: keep; TEST_HAS_FLAT_SET, flat_set
#include <test/minimal_words.hpp>                        // minimal_words
#include <test/uint128.hpp>                              // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_bounded_set.hpp>                 // basic_bit_bounded_set
#include <xstd/bits/bit_fixed_set.hpp>                   // basic_bit_fixed_set
#include <xstd/bits/bit_set.hpp>                         // basic_bit_set
#include <xstd/bits/detail/contiguous_bit_container.hpp> // contiguous_bit_container
#include <xstd/bits/detail/set_adaptor.hpp>              // set_adaptor
#include <xstd/bits/ext/boost/bit_small_set.hpp>         // basic_bit_small_set
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t, uint16_t, uint32_t, uint64_t
#include <set>                                           // set
#include <tuple>                                         // tuple, tuple_cat
#include <utility>                                       // declval

// The candidates for the set reading, the standard library's models first, graded by what a sweep can afford.
namespace test::spec::set {

using models = std::tuple<std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET

                          ,
                          std::flat_set<std::size_t>

#endif
                          >;

using dynamic = std::tuple<xstd::basic_bit_set<std::uint8_t>, xstd::basic_bit_set<std::uint64_t>>;

// Storage written outside the library, adapted by the same set adaptor the owners derive from.
using user_storage = std::tuple<xstd::bits::detail::set_adaptor<xstd::bits::detail::contiguous_bit_container<test::minimal_words<std::uint8_t>>>>;

// Every Block at an empty width, a single bit, and either side of its first two block boundaries.
using fixed_every_width = std::tuple<xstd::basic_bit_fixed_set<std::uint8_t, 0>, xstd::basic_bit_fixed_set<std::uint8_t, 1>, xstd::basic_bit_fixed_set<std::uint8_t, 7>, xstd::basic_bit_fixed_set<std::uint8_t, 8>, xstd::basic_bit_fixed_set<std::uint8_t, 9>, xstd::basic_bit_fixed_set<std::uint8_t, 15>, xstd::basic_bit_fixed_set<std::uint8_t, 16>, xstd::basic_bit_fixed_set<std::uint8_t, 17>, xstd::basic_bit_fixed_set<std::uint8_t, 24>, xstd::basic_bit_fixed_set<std::uint16_t, 0>, xstd::basic_bit_fixed_set<std::uint16_t, 1>, xstd::basic_bit_fixed_set<std::uint16_t, 15>, xstd::basic_bit_fixed_set<std::uint16_t, 16>, xstd::basic_bit_fixed_set<std::uint16_t, 17>, xstd::basic_bit_fixed_set<std::uint16_t, 31>, xstd::basic_bit_fixed_set<std::uint16_t, 32>, xstd::basic_bit_fixed_set<std::uint16_t, 33>, xstd::basic_bit_fixed_set<std::uint16_t, 48>, xstd::basic_bit_fixed_set<std::uint32_t, 0>, xstd::basic_bit_fixed_set<std::uint32_t, 1>, xstd::basic_bit_fixed_set<std::uint32_t, 31>, xstd::basic_bit_fixed_set<std::uint32_t, 32>, xstd::basic_bit_fixed_set<std::uint32_t, 33>, xstd::basic_bit_fixed_set<std::uint32_t, 63>, xstd::basic_bit_fixed_set<std::uint32_t, 64>, xstd::basic_bit_fixed_set<std::uint32_t, 65>, xstd::basic_bit_fixed_set<std::uint64_t, 0>, xstd::basic_bit_fixed_set<std::uint64_t, 1>, xstd::basic_bit_fixed_set<std::uint64_t, 63>, xstd::basic_bit_fixed_set<std::uint64_t, 64>, xstd::basic_bit_fixed_set<std::uint64_t, 65>
#ifdef TEST_HAS_UINT128

                                     ,
                                     xstd::basic_bit_fixed_set<xstd::uint128, 0>, xstd::basic_bit_fixed_set<xstd::uint128, 1>, xstd::basic_bit_fixed_set<xstd::uint128, 127>, xstd::basic_bit_fixed_set<xstd::uint128, 128>, xstd::basic_bit_fixed_set<xstd::uint128, 129>

#endif
                                     >;

using bounded_every_width = std::tuple<xstd::basic_bit_bounded_set<std::uint8_t, 0>, xstd::basic_bit_bounded_set<std::uint8_t, 9>, xstd::basic_bit_bounded_set<std::uint8_t, 17>, xstd::basic_bit_bounded_set<std::uint64_t, 65>>;

using small_every_width = std::tuple<xstd::basic_bit_small_set<std::uint8_t, 9>, xstd::basic_bit_small_set<std::uint64_t, 64>>;

// The narrowest Block across two boundaries, and every other Block at one width that spans three narrow blocks.
using fixed_boundary_widths = std::tuple<xstd::basic_bit_fixed_set<std::uint8_t, 0>, xstd::basic_bit_fixed_set<std::uint8_t, 1>, xstd::basic_bit_fixed_set<std::uint8_t, 8>, xstd::basic_bit_fixed_set<std::uint8_t, 9>, xstd::basic_bit_fixed_set<std::uint8_t, 16>, xstd::basic_bit_fixed_set<std::uint8_t, 17>, xstd::basic_bit_fixed_set<std::uint8_t, 24>, xstd::basic_bit_fixed_set<std::uint16_t, 24>, xstd::basic_bit_fixed_set<std::uint32_t, 24>, xstd::basic_bit_fixed_set<std::uint64_t, 24>
#ifdef TEST_HAS_UINT128

                                         ,
                                         xstd::basic_bit_fixed_set<xstd::uint128, 24>

#endif
                                         >;

using bounded_boundary_widths = std::tuple<xstd::basic_bit_bounded_set<std::uint8_t, 0>, xstd::basic_bit_bounded_set<std::uint8_t, 9>, xstd::basic_bit_bounded_set<std::uint8_t, 17>, xstd::basic_bit_bounded_set<std::uint8_t, 24>, xstd::basic_bit_bounded_set<std::uint64_t, 24>>;

// The inline blocks hold the first 16 keys, so a sweep to its limit spills onto the heap.
using small_boundary_widths = std::tuple<xstd::basic_bit_small_set<std::uint8_t, 9>>;

// Either side of the first byte boundary, and every Block at one width past it.
using fixed_few_widths = std::tuple<xstd::basic_bit_fixed_set<std::uint8_t, 0>, xstd::basic_bit_fixed_set<std::uint8_t, 8>, xstd::basic_bit_fixed_set<std::uint8_t, 9>, xstd::basic_bit_fixed_set<std::uint8_t, 17>, xstd::basic_bit_fixed_set<std::uint16_t, 17>, xstd::basic_bit_fixed_set<std::uint32_t, 17>, xstd::basic_bit_fixed_set<std::uint64_t, 17>
#ifdef TEST_HAS_UINT128

                                    ,
                                    xstd::basic_bit_fixed_set<xstd::uint128, 17>

#endif
                                    >;

using bounded_few_widths = std::tuple<xstd::basic_bit_bounded_set<std::uint8_t, 9>, xstd::basic_bit_bounded_set<std::uint8_t, 17>>;

using small_few_widths = std::tuple<xstd::basic_bit_small_set<std::uint8_t, 9>>;

// Widths no exhaustive sweep reaches, each off a block boundary of its own Block or of the byte.
using fixed_random_widths = std::tuple<xstd::basic_bit_fixed_set<std::uint8_t, 257>, xstd::basic_bit_fixed_set<std::uint32_t, 1023>, xstd::basic_bit_fixed_set<std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                                       ,
                                       xstd::basic_bit_fixed_set<xstd::uint128, 2049>

#endif
                                       >;

using bounded_random_widths = std::tuple<xstd::basic_bit_bounded_set<std::uint64_t, 4097>>;

// The inline blocks hold half of what a sample spans, so the dense samples spill onto the heap.
using small_random_widths = std::tuple<xstd::basic_bit_small_set<std::uint64_t, 1024>>;

// The widest grid, for the sweeps that are constant per type.
using every_width = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed_every_width>(), std::declval<dynamic>(), std::declval<bounded_every_width>(), std::declval<small_every_width>(), std::declval<user_storage>()));

// For the sweeps linear or quadratic in the width.
using boundary_widths = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed_boundary_widths>(), std::declval<dynamic>(), std::declval<bounded_boundary_widths>(), std::declval<small_boundary_widths>(), std::declval<user_storage>()));

// For the sweeps cubic or quartic in the width.
using few_widths = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed_few_widths>(), std::declval<dynamic>(), std::declval<bounded_few_widths>(), std::declval<small_few_widths>(), std::declval<user_storage>()));

// For the sampled sweeps, whose cost is the sample count rather than the width.
using random_widths = decltype(std::tuple_cat(std::declval<models>(), std::declval<fixed_random_widths>(), std::declval<dynamic>(), std::declval<bounded_random_widths>(), std::declval<small_random_widths>(), std::declval<user_storage>()));

} // namespace test::spec::set

#endif // TEST_SPEC_SET_HPP
