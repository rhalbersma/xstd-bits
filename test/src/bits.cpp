//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// The two vehicle headers are named directly because the assertions below pin the three naming layers to their
// storage, and the umbrella stopped exporting those names when the vehicles moved under detail/. A test may
// reach into detail/ where a user may not, and an include list is where that is said out loud.
// [design.md#the-interface-line]
#include <test/block_types.hpp>                       // graded_extents
#include <test/flat_set.hpp>                          // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/inplace_vector.hpp>                    // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/sequence/concepts.hpp>                 // bit_sequence
#include <test/set/concepts.hpp>                      // bit_set
#include <xstd/bits.hpp>                              // the whole bits surface
#include <xstd/bits/detail/contiguous_bit_array.hpp>  // contiguous_bit_array
#include <xstd/bits/detail/contiguous_bit_vector.hpp> // contiguous_bit_vector
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE
#include <array>                                      // array
#include <concepts>                                   // same_as
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint8_t
#include <limits>                                     // numeric_limits
#include <memory>                                     // allocator
#include <ranges>                                     // bidirectional_range, random_access_range
#include <set>                                        // set
#include <tuple>                                      // tuple_element_t, tuple_size_v
#include <utility>                                    // index_sequence, make_index_sequence


// Every entity the umbrella promises, reached through it alone: no leaf test sees the umbrella at all.
BOOST_AUTO_TEST_CASE(EveryContainerArrivesThroughTheUmbrella)
{
        // The two containers that are ranges on their own terms: one indexed by position, one iterating its elements.
        static_assert(std::ranges::random_access_range<xstd::bit_array<8>>);
        static_assert(std::ranges::bidirectional_range<xstd::bit_static_set<8>>);

        // xstd::bitset is deliberately not a range, reproducing std::bitset, so the trait has to deliver a working view over it.
        static_assert(not std::ranges::range<xstd::bitset<8>>);

        auto const legacy = xstd::bitset<8>();
        static_assert(std::ranges::bidirectional_range<decltype(xstd::bit_set_view(legacy))>);

        auto const packed = xstd::bit_array<8>();
        static_assert(std::ranges::random_access_range<decltype(xstd::bit_span(packed))>);

        // The dynamic column, one name per reading, all three over a contiguous_bit_vector.
        static_assert(std::ranges::bidirectional_range<xstd::basic_bit_set<std::size_t>>);
        static_assert(std::ranges::random_access_range<xstd::basic_bit_vector<std::size_t>>);
        static_assert(not std::ranges::range<xstd::basic_dynamic_bitset<std::size_t>>);

        // Three layers: the primaries take the storage, the basic_ layer chooses it and leaves the block open, the restricted layer fixes size_t and std::allocator. [design.md#the-public-names]
        static_assert(std::same_as<xstd::basic_bit_static_set<std::uint8_t, 8>, xstd::set_adaptor<xstd::detail::bits::contiguous_bit_array<std::uint8_t, 8>, xstd::ownership::owns>>);
        static_assert(std::same_as<xstd::basic_bit_set<std::uint8_t>,          xstd::set_adaptor<xstd::detail::bits::contiguous_bit_vector<std::uint8_t>, xstd::ownership::owns>>);
        static_assert(std::same_as<xstd::bit_static_set<8>, xstd::basic_bit_static_set<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_array<8>,      xstd::basic_bit_array<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bitset<8>,         xstd::basic_bitset<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_set,        xstd::basic_bit_set<std::size_t, std::allocator<std::size_t>>>);
        static_assert(std::same_as<xstd::bit_vector,     xstd::basic_bit_vector<std::size_t, std::allocator<std::size_t>>>);
        static_assert(std::same_as<xstd::dynamic_bitset, xstd::basic_dynamic_bitset<std::size_t, std::allocator<std::size_t>>>);

#ifdef TEST_HAS_INPLACE_VECTOR
        // The inplace column, the third storage point, one name per reading and every one of them an alias like the rest. [design.md#the-inplace-column]
        static_assert(std::ranges::bidirectional_range<xstd::basic_bit_inplace_set<std::uint8_t, 8>>);
        static_assert(std::ranges::random_access_range<xstd::basic_bit_inplace_vector<std::uint8_t, 8>>);
        static_assert(not std::ranges::range<xstd::basic_inplace_bitset<std::uint8_t, 8>>);
        static_assert(std::same_as<xstd::bit_inplace_set<8>,    xstd::basic_bit_inplace_set<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_inplace_vector<8>, xstd::basic_bit_inplace_vector<std::size_t, 8>>);
        static_assert(std::same_as<xstd::inplace_bitset<8>,     xstd::basic_inplace_bitset<std::size_t, 8>>);
#endif

        // Every static name has an aligned form in both layers, its width rounded up to whole blocks; the inplace column has none, its capacity already being whole blocks. [design.md#the-public-names]
        static_assert(std::same_as<xstd::aligned::bit_static_set<9>, xstd::bit_static_set<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bit_array<9>,      xstd::bit_array<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::bitset<9>,         xstd::bitset<std::numeric_limits<std::size_t>::digits>>);
        static_assert(std::same_as<xstd::aligned::basic_bitset<std::uint8_t, 9>, xstd::basic_bitset<std::uint8_t, 16>>);
        static_assert(std::same_as<xstd::aligned::basic_bitset<std::uint8_t, 0>, xstd::basic_bitset<std::uint8_t, 0>>);
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
        using packed = test::graded_extents<xstd::basic_bit_array>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_sequence<std::tuple_element_t<I, packed>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<packed>>{});

#ifdef TEST_HAS_INPLACE_VECTOR
        // Storage is the second dimension of the grading: the same claim over the same extents, read as capacities. [design.md#the-inplace-column]
        using inplace = test::graded_extents<xstd::basic_bit_inplace_vector>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_sequence<std::tuple_element_t<I, inplace>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<inplace>>{});
#endif
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

        using packed = test::graded_extents<xstd::basic_bit_static_set>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_set<std::tuple_element_t<I, packed>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<packed>>{});

#ifdef TEST_HAS_INPLACE_VECTOR
        // And the same second dimension on this reading. [design.md#the-inplace-column]
        using inplace = test::graded_extents<xstd::basic_bit_inplace_set>;
        [] <std::size_t... I> (std::index_sequence<I...>) {
                static_assert((bit_set<std::tuple_element_t<I, inplace>> and ...));
        }(std::make_index_sequence<std::tuple_size_v<inplace>>{});
#endif
}
