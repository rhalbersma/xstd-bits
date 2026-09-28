//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_SEQUENCE_HPP
#define TEST_SPEC_SEQUENCE_HPP

#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/uint128.hpp>                         // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <array>                                    // array
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                    // tuple, tuple_cat
#include <utility>                                  // declval
#include <vector>                                   // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

// The candidates for the sequence reading, each column's model first, graded by what a sweep can afford.
namespace test::spec::sequence {

// std::inplace_vector<bool, N> at the given capacities where the standard library has it, and nothing where not.
template<std::size_t... N>
using inplace_models = std::tuple<
#ifdef TEST_HAS_INPLACE_VECTOR

        std::inplace_vector<bool, N>...

#endif
        >;

// The column [vector.bool] describes: the model, the dynamic sequences, and the small one over Boost's storage.
using vector_model = std::tuple<std::vector<bool>>;

using dynamic = std::tuple<xstd::basic_bit_vector<std::uint8_t>, xstd::basic_bit_vector<std::uint64_t>>;

// Every Block at an empty width, a single bit, and either side of a block boundary.
using fixed_every_width = std::tuple<xstd::basic_bit_array<std::uint8_t, 0>, xstd::basic_bit_array<std::uint8_t, 1>, xstd::basic_bit_array<std::uint8_t, 7>, xstd::basic_bit_array<std::uint8_t, 8>, xstd::basic_bit_array<std::uint8_t, 9>, xstd::basic_bit_array<std::uint8_t, 17>, xstd::basic_bit_array<std::uint8_t, 24>, xstd::basic_bit_array<std::uint16_t, 15>, xstd::basic_bit_array<std::uint16_t, 16>, xstd::basic_bit_array<std::uint16_t, 17>, xstd::basic_bit_array<std::uint32_t, 31>, xstd::basic_bit_array<std::uint32_t, 32>, xstd::basic_bit_array<std::uint32_t, 33>, xstd::basic_bit_array<std::uint64_t, 0>, xstd::basic_bit_array<std::uint64_t, 63>, xstd::basic_bit_array<std::uint64_t, 64>, xstd::basic_bit_array<std::uint64_t, 65>
#ifdef TEST_HAS_UINT128

                                     ,
                                     xstd::basic_bit_array<xstd::uint128, 127>, xstd::basic_bit_array<xstd::uint128, 128>, xstd::basic_bit_array<xstd::uint128, 129>

#endif
                                     >;

using array_models_every_width = std::tuple<std::array<bool, 0>, std::array<bool, 1>, std::array<bool, 8>, std::array<bool, 9>, std::array<bool, 64>, std::array<bool, 65>>;

using bounded_every_width = std::tuple<xstd::basic_bit_bounded_vector<std::uint8_t, 0>, xstd::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 17>, xstd::basic_bit_bounded_vector<std::uint64_t, 65>>;

using inplace_models_every_width = inplace_models<0, 9, 65>;

using small_every_width = std::tuple<xstd::basic_bit_small_vector<std::uint8_t, 9>, xstd::basic_bit_small_vector<std::uint64_t, 64>>;

// The narrowest Block across two boundaries, and every other Block at one width that spans three narrow blocks.
using fixed_boundary_widths = std::tuple<xstd::basic_bit_array<std::uint8_t, 0>, xstd::basic_bit_array<std::uint8_t, 1>, xstd::basic_bit_array<std::uint8_t, 8>, xstd::basic_bit_array<std::uint8_t, 9>, xstd::basic_bit_array<std::uint8_t, 16>, xstd::basic_bit_array<std::uint8_t, 17>, xstd::basic_bit_array<std::uint8_t, 24>, xstd::basic_bit_array<std::uint16_t, 24>, xstd::basic_bit_array<std::uint32_t, 24>, xstd::basic_bit_array<std::uint64_t, 24>
#ifdef TEST_HAS_UINT128

                                         ,
                                         xstd::basic_bit_array<xstd::uint128, 24>

#endif
                                         >;

using array_models_boundary_widths = std::tuple<std::array<bool, 0>, std::array<bool, 9>, std::array<bool, 24>>;

using bounded_boundary_widths = std::tuple<xstd::basic_bit_bounded_vector<std::uint8_t, 0>, xstd::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 17>, xstd::basic_bit_bounded_vector<std::uint8_t, 24>, xstd::basic_bit_bounded_vector<std::uint64_t, 24>>;

using inplace_models_boundary_widths = inplace_models<9, 24>;

// The inline blocks hold the first 16 positions, so a sweep to its limit spills onto the heap.
using small_boundary_widths = std::tuple<xstd::basic_bit_small_vector<std::uint8_t, 9>>;

// Either side of the first byte boundary, and every Block at one width past it.
using fixed_few_widths = std::tuple<xstd::basic_bit_array<std::uint8_t, 0>, xstd::basic_bit_array<std::uint8_t, 8>, xstd::basic_bit_array<std::uint8_t, 9>, xstd::basic_bit_array<std::uint8_t, 17>, xstd::basic_bit_array<std::uint16_t, 17>, xstd::basic_bit_array<std::uint32_t, 17>, xstd::basic_bit_array<std::uint64_t, 17>
#ifdef TEST_HAS_UINT128

                                    ,
                                    xstd::basic_bit_array<xstd::uint128, 17>

#endif
                                    >;

using array_models_few_widths = std::tuple<std::array<bool, 0>, std::array<bool, 17>>;

using bounded_few_widths = std::tuple<xstd::basic_bit_bounded_vector<std::uint8_t, 9>, xstd::basic_bit_bounded_vector<std::uint8_t, 17>>;

using inplace_models_few_widths = inplace_models<17>;

using small_few_widths = std::tuple<xstd::basic_bit_small_vector<std::uint8_t, 9>>;

// Widths no exhaustive sweep reaches, each off a block boundary of its own Block or of the byte.
using fixed_random_widths = std::tuple<xstd::basic_bit_array<std::uint8_t, 257>, xstd::basic_bit_array<std::uint32_t, 1023>, xstd::basic_bit_array<std::uint64_t, 1025>
#ifdef TEST_HAS_UINT128

                                       ,
                                       xstd::basic_bit_array<xstd::uint128, 2049>

#endif
                                       >;

using array_models_random_widths = std::tuple<std::array<bool, 1025>>;

using bounded_random_widths = std::tuple<xstd::basic_bit_bounded_vector<std::uint64_t, 4097>>;

using inplace_models_random_widths = inplace_models<4097>;

// The inline blocks hold half of what a sample spans, so the dense samples spill onto the heap.
using small_random_widths = std::tuple<xstd::basic_bit_small_vector<std::uint64_t, 1024>>;

// [array]'s column: std::array<bool, N> and the fixed sequences.
using array_every_width = decltype(std::tuple_cat(std::declval<array_models_every_width>(), std::declval<fixed_every_width>()));
using array_boundary_widths = decltype(std::tuple_cat(std::declval<array_models_boundary_widths>(), std::declval<fixed_boundary_widths>()));
using array_few_widths = decltype(std::tuple_cat(std::declval<array_models_few_widths>(), std::declval<fixed_few_widths>()));
using array_random_widths = decltype(std::tuple_cat(std::declval<array_models_random_widths>(), std::declval<fixed_random_widths>()));

// [vector.bool]'s column: std::vector<bool>, the dynamic sequences and the small ones.
using vector_every_width = decltype(std::tuple_cat(std::declval<vector_model>(), std::declval<dynamic>(), std::declval<small_every_width>()));
using vector_boundary_widths = decltype(std::tuple_cat(std::declval<vector_model>(), std::declval<dynamic>(), std::declval<small_boundary_widths>()));
using vector_few_widths = decltype(std::tuple_cat(std::declval<vector_model>(), std::declval<dynamic>(), std::declval<small_few_widths>()));
using vector_random_widths = decltype(std::tuple_cat(std::declval<vector_model>(), std::declval<dynamic>(), std::declval<small_random_widths>()));

// [inplace.vector]'s column: std::inplace_vector<bool, N> where the library has it, and the bounded sequences.
using inplace_vector_every_width = decltype(std::tuple_cat(std::declval<inplace_models_every_width>(), std::declval<bounded_every_width>()));
using inplace_vector_boundary_widths = decltype(std::tuple_cat(std::declval<inplace_models_boundary_widths>(), std::declval<bounded_boundary_widths>()));
using inplace_vector_few_widths = decltype(std::tuple_cat(std::declval<inplace_models_few_widths>(), std::declval<bounded_few_widths>()));
using inplace_vector_random_widths = decltype(std::tuple_cat(std::declval<inplace_models_random_widths>(), std::declval<bounded_random_widths>()));

// The two columns that grow, for [sequence.reqmts]'s modifiers.
using growable_every_width = decltype(std::tuple_cat(std::declval<vector_every_width>(), std::declval<inplace_vector_every_width>()));
using growable_boundary_widths = decltype(std::tuple_cat(std::declval<vector_boundary_widths>(), std::declval<inplace_vector_boundary_widths>()));
using growable_few_widths = decltype(std::tuple_cat(std::declval<vector_few_widths>(), std::declval<inplace_vector_few_widths>()));
using growable_random_widths = decltype(std::tuple_cat(std::declval<vector_random_widths>(), std::declval<inplace_vector_random_widths>()));

// The widest grid, for the sweeps that are constant per type.
using every_width = decltype(std::tuple_cat(std::declval<array_every_width>(), std::declval<growable_every_width>()));

// For the sweeps linear or quadratic in the width.
using boundary_widths = decltype(std::tuple_cat(std::declval<array_boundary_widths>(), std::declval<growable_boundary_widths>()));

// For the sweeps cubic in the width.
using few_widths = decltype(std::tuple_cat(std::declval<array_few_widths>(), std::declval<growable_few_widths>()));

// For the sampled sweeps, whose cost is the sample count rather than the width.
using random_widths = decltype(std::tuple_cat(std::declval<array_random_widths>(), std::declval<growable_random_widths>()));

} // namespace test::spec::sequence

#endif // TEST_SPEC_SEQUENCE_HPP
