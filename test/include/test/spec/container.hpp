//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_CONTAINER_HPP
#define TEST_SPEC_CONTAINER_HPP

#include <test/flat_set.hpp>                        // IWYU pragma: keep; TEST_HAS_FLAT_SET, flat_set
#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_words.hpp>                   // minimal_words
#include <test/spec/sequence.hpp>                   // all, pairs, sequences
#include <test/spec/set.hpp>                        // all, pairs, pairs_with_doubletons, sets
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/detail/bit_container.hpp>       // bit_container
#include <xstd/bits/detail/bounded_blocks.hpp>      // IWYU pragma: keep; XSTD_BITS_HAS_CONSTEXPR_BOUNDED
#include <xstd/bits/detail/set_adaptor.hpp>         // set_adaptor
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <cstddef>                                  // size_t
#include <set>                                      // set
#include <tuple>                                    // tuple_cat
#include <utility>                                  // declval

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

// The candidates for [container.requirements], which both the set and the sequence reading meet.
namespace test::spec::container {

using all = decltype(std::tuple_cat(std::declval<set::all>(), std::declval<sequence::all>()));

// A set names the key it holds, where a sequence holds bools by position.
template<class X>
concept keyed = requires { typename X::key_type; };

// Whose iterators are constexpr ones: every candidate but those over node, Boost or user storage.
template<class X>
inline constexpr auto constant_evaluable_v = true;

template<class Key, class Compare, class Allocator>
inline constexpr auto constant_evaluable_v<std::set<Key, Compare, Allocator>> = false;

#ifdef TEST_HAS_FLAT_SET

template<class Key, class Compare, class KeyContainer>
inline constexpr auto constant_evaluable_v<std::flat_set<Key, Compare, KeyContainer>> = false;

#endif

template<class Block, std::size_t N, class Allocator>
inline constexpr auto constant_evaluable_v<xstd::basic_bit_small_set<Block, N, Allocator>> = false;

template<class Block, std::size_t N, class Allocator>
inline constexpr auto constant_evaluable_v<xstd::basic_bit_small_vector<Block, N, Allocator>> = false;

template<class Block>
inline constexpr auto constant_evaluable_v<xstd::bits::detail::set_adaptor<xstd::bits::detail::bit_container<test::minimal_words<Block>>>> = false;

#ifndef XSTD_BITS_HAS_CONSTEXPR_BOUNDED

template<class Block, std::size_t N>
inline constexpr auto constant_evaluable_v<xstd::basic_bit_bounded_set<Block, N>> = false;

template<class Block, std::size_t N>
inline constexpr auto constant_evaluable_v<xstd::basic_bit_bounded_vector<Block, N>> = false;

#endif

// The columns [inplace.vector] specifies, whose swap it gives an exception specification of its own.
template<class X>
inline constexpr auto bounded_v = false;

template<class Block, std::size_t N>
inline constexpr auto bounded_v<xstd::basic_bit_bounded_set<Block, N>> = true;

template<class Block, std::size_t N>
inline constexpr auto bounded_v<xstd::basic_bit_bounded_vector<Block, N>> = true;

#ifdef TEST_HAS_INPLACE_VECTOR

template<std::size_t N>
inline constexpr auto bounded_v<std::inplace_vector<bool, N>> = true;

#endif

// Each reading's inputs, so one case runs over the sets and the sequences alike.
namespace inputs {

template<class X>
[[nodiscard]] auto objects()
{
        if constexpr (keyed<X>) {
                return set::inputs::sets<X>();
        } else {
                return sequence::inputs::sequences<X>();
        }
}

template<class X>
[[nodiscard]] auto pairs()
{
        if constexpr (keyed<X>) {
                return set::inputs::pairs<X>();
        } else {
                return sequence::inputs::pairs<X>();
        }
}

// Every pair of doubletons for a set as well, so each way four keys interleave.
template<class X>
[[nodiscard]] auto pairs_with_doubletons()
{
        if constexpr (keyed<X>) {
                return set::inputs::pairs_with_doubletons<X>();
        } else {
                return sequence::inputs::pairs<X>();
        }
}

} // namespace inputs

} // namespace test::spec::container

#endif // TEST_SPEC_CONTAINER_HPP
