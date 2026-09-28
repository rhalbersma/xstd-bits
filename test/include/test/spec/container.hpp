//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SPEC_CONTAINER_HPP
#define TEST_SPEC_CONTAINER_HPP

#include <test/spec/sequence.hpp> // all, pairs, sequences
#include <test/spec/set.hpp>      // all, pairs, pairs_with_doubletons, sets
#include <tuple>                  // tuple_cat
#include <utility>                // declval

// The candidates for [container.requirements], which both the set and the sequence reading meet.
namespace test::spec::container {

using all = decltype(std::tuple_cat(std::declval<set::all>(), std::declval<sequence::all>()));

// A set names the key it holds, where a sequence holds bools by position.
template<class X>
concept keyed = requires { typename X::key_type; };

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
