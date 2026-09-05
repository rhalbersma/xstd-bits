//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <test/sequence/ordering.hpp>             // ordering_agrees_with_vector_bool
#include <xstd/bits/bit_array.hpp>                // bit_array
#include <xstd/bits/bit_static_set.hpp>           // bit_static_set
#include <xstd/bits/bitset.hpp>                   // sequence_find over xstd::bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // sequence_find over boost::dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // sequence_find over std::bitset
#include <xstd/bits/ranges/sequence_view.hpp>     // sequence_view, sequence_range
#include <algorithm>                              // equal
#include <array>                                  // array
#include <bitset>                                 // bitset
#include <ranges>                                 // random_access_range

BOOST_AUTO_TEST_SUITE(Ranges)
BOOST_AUTO_TEST_SUITE(SequenceView)

BOOST_AUTO_TEST_CASE(TheViewedTypesAreTheOnesHoldingBoolsWithoutOfferingThem)
{
        static_assert(xstd::ranges::sequence_range<std::bitset<8>>);
        static_assert(xstd::ranges::sequence_range<xstd::bitset<8>>);
        static_assert(xstd::ranges::sequence_range<boost::dynamic_bitset<>>);

        static_assert(std::ranges::random_access_range<xstd::sequence_view<std::bitset<8>>>);
        static_assert(std::ranges::random_access_range<xstd::sequence_view<xstd::bitset<8>>>);

        // bit_static_set is not a sequence of bools; it is a set of keys.
        static_assert(not xstd::ranges::sequence_range<xstd::bit_static_set<8>>);
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

                auto const view = xstd::sequence_view(packed);
                BOOST_CHECK_EQUAL(view.size(), N);
                BOOST_CHECK(std::ranges::equal(view, plain));
        }
}

// A view is mutable through: writing a position through the view writes the bit.
BOOST_AUTO_TEST_CASE(WritingThroughTheViewWritesTheBits)
{
        auto packed = xstd::bitset<8>();
        auto const view = xstd::sequence_view(packed);

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
        auto packed = xstd::bit_array<8, unsigned char>{};
        packed[1] = true;
        packed[6] = true;

        auto const view = xstd::sequence_view(packed);
        for (auto k = 0UZ; k < 8UZ; ++k) {
                BOOST_CHECK_EQUAL(static_cast<bool>(view[k]), static_cast<bool>(packed[k]));
        }

        // Both directions, through both proxies: only setting was exercised at first, leaving the clearing half of assign_at unreached.
        packed[1] = false;
        BOOST_CHECK(not static_cast<bool>(packed[1]));
        BOOST_CHECK(not static_cast<bool>(view[1]));

        view[6] = false;
        BOOST_CHECK(not static_cast<bool>(packed[6]));

        view[0] = true;
        BOOST_CHECK(static_cast<bool>(packed[0]));
}

// The sequence ordering against std::vector<bool>, over both routes: block-streaming where the type says where its blocks are, element-wise where not.
BOOST_AUTO_TEST_CASE(EveryViewedTypeOrdersLikeAVectorBool)
{
        test::sequence::ordering_agrees_with_vector_bool<xstd::bitset<8>>();
        test::sequence::ordering_agrees_with_vector_bool<std::bitset<8>>();
        test::sequence::ordering_agrees_with_vector_bool<boost::dynamic_bitset<>>();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
