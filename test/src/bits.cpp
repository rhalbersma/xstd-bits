//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE
#include <test/block_types.hpp>       // graded_extents
#include <test/flat_set.hpp>          // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/sequence/concepts.hpp> // bit_sequence
#include <test/set/concepts.hpp>      // bit_set
#include <xstd/bits.hpp>              // the whole bits surface
#include <array>                      // array
#include <concepts>                   // same_as
#include <cstddef>                    // size_t
#include <cstdint>                    // uint8_t
#include <limits>                     // numeric_limits
#include <ranges>                     // bidirectional_range, random_access_range
#include <set>                        // set
#include <tuple>                      // tuple_element_t, tuple_size_v
#include <utility>                    // index_sequence, make_index_sequence


// Every entity the umbrella promises, reached through it alone: no leaf test sees the umbrella at all.
BOOST_AUTO_TEST_CASE(EveryContainerArrivesThroughTheUmbrella)
{
        // The two containers that are ranges on their own terms: one indexed by position, one iterating its elements.
        static_assert(std::ranges::random_access_range<xstd::bit_array<8>>);
        static_assert(std::ranges::bidirectional_range<xstd::bit_static_set<8>>);

        // xstd::bitset is deliberately not a range, reproducing std::bitset, so the trait has to deliver a working view over it.
        static_assert(not std::ranges::range<xstd::bitset<8>>);

        auto const legacy = xstd::bitset<8>();
        static_assert(std::ranges::bidirectional_range<decltype(xstd::set_view(legacy))>);

        auto const packed = xstd::bit_array<8>();
        static_assert(std::ranges::random_access_range<decltype(xstd::sequence_view(packed))>);

        // The dynamic column, one name per reading, all three over a block_vector.
        static_assert(std::ranges::bidirectional_range<xstd::bit_set<std::size_t>>);
        static_assert(std::ranges::random_access_range<xstd::bit_vector<std::size_t>>);
        static_assert(not std::ranges::range<xstd::dynamic_bitset<std::size_t>>);

        // Every public name takes size_t as its block unless told otherwise, and every static name has an aligned form, its width rounded up to whole blocks. [design.md#the-public-names]
        static_assert(std::same_as<xstd::bit_set<>,        xstd::bit_set<std::size_t>>);
        static_assert(std::same_as<xstd::bit_vector<>,     xstd::bit_vector<std::size_t>>);
        static_assert(std::same_as<xstd::dynamic_bitset<>, xstd::dynamic_bitset<std::size_t>>);
        static_assert(std::same_as<xstd::aligned::bit_static_set<9>, xstd::bit_static_set<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bit_array<9>,      xstd::bit_array<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bitset<9>,         xstd::bitset<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bitset<9, std::uint8_t>, xstd::bitset<16, std::uint8_t>>);
        static_assert(std::same_as<xstd::aligned::bitset<0, std::uint8_t>, xstd::bitset< 0, std::uint8_t>>);
}

// A packed container satisfies the same interface as the one it packs, which means something only because std::array answers to it too.
BOOST_AUTO_TEST_CASE(APackedArrayIsTheArrayItPacks)
{
        using namespace test::sequence;

        // The standard's side, at the extents a packed array grades over.
        static_assert(bit_sequence<std::array<bool,  0>>);
        static_assert(bit_sequence<std::array<bool,  1>>);
        static_assert(bit_sequence<std::array<bool,  8>>);
        static_assert(bit_sequence<std::array<bool, 64>>);

        // And ours, over every Block model and extent the grading names.
        using packed = test::graded_extents<xstd::bit_array>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_sequence<std::tuple_element_t<I, packed>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<packed>>{});
}

// The same claim on the other reading: a set of keys and a sequence of bools are different interfaces.
BOOST_AUTO_TEST_CASE(APackedSetIsTheSetItPacks)
{
        using namespace test::set;

        // std::flat_set for the reason std::array is above: a second reference keeps the concept from describing one implementation.
        static_assert(bit_set<std::set<std::size_t>>);
#ifdef TEST_HAS_FLAT_SET
        static_assert(bit_set<std::flat_set<std::size_t>>);
#endif

        using packed = test::graded_extents<xstd::bit_static_set>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_set<std::tuple_element_t<I, packed>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<packed>>{});
}
