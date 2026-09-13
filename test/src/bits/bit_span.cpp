//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/ordering.hpp>                // ordering_agrees_with_vector_bool
#include <xstd/bits/bit_array.hpp>                   // bit_array
#include <xstd/bits/bit_span.hpp>                    // bit_span
#include <xstd/bits/bit_static_set.hpp>              // bit_static_set
#include <xstd/bits/bitset.hpp>                      // bitset
#include <xstd/bits/detail/contiguous_bit_array.hpp> // contiguous_bit_array
#include <xstd/bits/dynamic_bitset.hpp>             // dynamic_bitset
#include <xstd/bits/ownership.hpp>                   // ownership
#include <xstd/bits/sequence_adaptor.hpp>            // sequence_adaptor
#include <boost/test/unit_test.hpp>                  // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <algorithm>                                 // equal
#include <array>                                     // array
#include <bitset>                                    // bitset
#include <concepts>                                  // derived_from, equality_comparable, same_as, totally_ordered
#include <cstddef>                                   // size_t
#include <cstdint>                                   // uint8_t
#include <ranges>                                    // borrowed_range, random_access_range, view
#include <utility>                                   // declval

BOOST_AUTO_TEST_SUITE(BitSpan)

namespace {

using Blocks = xstd::detail::bits::contiguous_bit_array<std::size_t, 8>;

template<class T>
using view_of = decltype(xstd::bit_span(std::declval<T&>()));

}  // namespace

// The view is the referring adaptor under another name, and over an owner of either reading it refers into the storage the owner wraps. [design.md#the-views-are-the-adaptors]
BOOST_AUTO_TEST_CASE(TheViewIsTheReferringAdaptor)
{
        static_assert(std::derived_from<xstd::bit_span<Blocks>, xstd::sequence_adaptor<Blocks, xstd::ownership::refers, false>>);
        static_assert(std::same_as<view_of<Blocks>,                  xstd::bit_span<Blocks>>);
        static_assert(std::same_as<view_of<Blocks const>,            xstd::bit_span<Blocks const>>);
        static_assert(std::same_as<view_of<xstd::bitset<8>>,         xstd::bit_span<xstd::detail::bits::contiguous_bit_array<std::size_t, 8>>>);
        static_assert(std::same_as<view_of<xstd::bit_static_set<8>>, xstd::bit_span<xstd::detail::bits::contiguous_bit_array<std::size_t, 8>>>);
}

BOOST_AUTO_TEST_CASE(TheViewedTypesAreTheOnesHoldingBoolsWithoutOfferingThem)
{
        static_assert(std::ranges::random_access_range<view_of<Blocks>>);
        static_assert(std::ranges::random_access_range<view_of<xstd::bitset<8>>>);
        static_assert(std::ranges::random_access_range<view_of<xstd::bit_array<8>>>);

        // A view in std::ranges' sense and borrowed, like span; and like span it neither compares nor orders. [design.md#views-follow-their-precedent]
        static_assert(std::ranges::view<view_of<Blocks>>);
        static_assert(std::ranges::borrowed_range<view_of<Blocks>>);
        static_assert(not std::equality_comparable<view_of<Blocks>>);
        static_assert(not std::totally_ordered<view_of<Blocks>>);
}

// The sequence reading is the bools at every position, checked against the std::array<bool, N> holding the same bits.
BOOST_AUTO_TEST_CASE(TheSequenceReadingIsTheArrayOfBools)
{
        constexpr auto N = 8UZ;
        for (auto i = 0UZ; i < (1UZ << N); ++i) {
                auto packed = xstd::bitset<N>();
                auto plain  = std::array<bool, N>{};
                for (auto k = 0UZ; k < N; ++k) {
                        if ((i >> k & 1UZ) != 0UZ) { packed.set(k); plain[k] = true; }
                }

                auto const view = xstd::bit_span(packed);
                BOOST_CHECK_EQUAL(view.size(), N);
                BOOST_CHECK(std::ranges::equal(view, plain));
        }
}

// A view is mutable through: writing a position through the view writes the bit.
BOOST_AUTO_TEST_CASE(WritingThroughTheViewWritesTheBits)
{
        auto packed = xstd::bitset<8>();
        auto const view = xstd::bit_span(packed);

        view[3] = true;
        BOOST_CHECK(packed.test(3));
        BOOST_CHECK_EQUAL(packed.count(), 1);

        view[3] = false;
        BOOST_CHECK(not packed.test(3));
        BOOST_CHECK(packed.none());
}

// The same reading over the type this library packs, so bit_array's own operator[] and the view agree position by position.
BOOST_AUTO_TEST_CASE(APackedArrayAgreesWithItsOwnView)
{
        auto packed = xstd::basic_bit_array<unsigned char, 8>{};
        packed[1] = true;
        packed[6] = true;

        auto const view = xstd::bit_span(packed);
        for (auto k = 0UZ; k < 8UZ; ++k) {
                BOOST_CHECK_EQUAL(static_cast<bool>(view[k]), static_cast<bool>(packed[k]));
        }

        // Both directions, through both proxies.
        packed[1] = false;
        BOOST_CHECK(not static_cast<bool>(packed[1]));
        BOOST_CHECK(not static_cast<bool>(view[1]));

        view[6] = false;
        BOOST_CHECK(not static_cast<bool>(packed[6]));

        view[0] = true;
        BOOST_CHECK(static_cast<bool>(packed[0]));
}

// The sequence reading against std::vector<bool>, through the iterators the view hands out, for every viewed type.
BOOST_AUTO_TEST_CASE(EveryViewedTypeReadsLikeAVectorBool)
{
        test::sequence::ordering_agrees_with_vector_bool<xstd::bitset<8>>();
        test::sequence::ordering_agrees_with_vector_bool<xstd::basic_bitset<std::uint8_t, 8>>();
        test::sequence::ordering_agrees_with_vector_bool<xstd::dynamic_bitset>();
}

BOOST_AUTO_TEST_SUITE_END()
