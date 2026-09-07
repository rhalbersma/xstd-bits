//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/bitset/exhaustive.hpp>             // all_singleton_sets, all_doubleton_sets, all_triplet_sets, empty_set, full_set
#include <test/bitset/primitives.hpp>             // mem_compare_three_way
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset_adaptor.hpp>             // bitset_adaptor
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <xstd/bits/ext/xstd/bitset.hpp>          // bitset
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
,        xstd::bitset_adaptor<std::bitset< 0>>
,        xstd::bitset_adaptor<std::bitset<17>>
,        xstd::basic_bitset< 0, uint8_t>
,        xstd::basic_bitset< 8, uint8_t>
,        xstd::basic_bitset< 9, uint8_t>
,        xstd::basic_bitset<17, uint8_t>
,        xstd::basic_bitset<17, uint16_t>
,        xstd::basic_bitset<17, uint32_t>
,        xstd::basic_bitset<17, uint64_t>
#ifdef TEST_HAS_UINT128
,        xstd::basic_bitset<17, xstd::uint128>
#endif
,        xstd::bitset_adaptor<boost::dynamic_bitset<>>
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
