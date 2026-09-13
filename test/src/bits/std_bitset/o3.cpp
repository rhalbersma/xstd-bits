//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp>             // all_singleton_sets, all_doubleton_sets, all_triplet_sets, empty_set, full_set
#include <test/bitset/primitives.hpp>             // mem_compare_three_way
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <boost/dynamic_bitset.hpp>              // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <bitset>                                 // bitset
#include <cstdint>                                // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(O3)

using namespace test::bitset;

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset< 0>
,         std::bitset<17>
,        xstd::basic_bitset<uint8_t, 0>
,        xstd::basic_bitset<uint8_t, 8>
,        xstd::basic_bitset<uint8_t, 9>
,        xstd::basic_bitset<uint8_t, 17>
,        xstd::basic_bitset<uint16_t, 17>
,        xstd::basic_bitset<uint32_t, 17>
,        xstd::basic_bitset<uint64_t, 17>
#ifdef TEST_HAS_UINT128
,        xstd::basic_bitset<xstd::uint128, 17>
#endif
,        xstd::basic_dynamic_bitset<uint8_t>
,        xstd::basic_dynamic_bitset<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareThreeWayHoldsOverEveryTripletAndDoubleton, T, Types)
{
        // empty/full set vs. every triplet at matching N -- see o1.cpp for why N is pinned.
        on3::all_triplet_sets<T>([](auto const& bs3) {
                on0::empty_set<T, limit_v<T, L3>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs3);
                });
                on0::full_set<T, limit_v<T, L3>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs3);
                });
        });

        // every singleton vs. every doubleton, at matching N.
        on2::all_doubleton_sets<T, limit_v<T, L3>>([](auto const& bs2) {
                on1::all_singleton_sets<T, limit_v<T, L3>>([&](auto const& bs1) {
                        mem_compare_three_way()(bs1, bs2);
                });
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
