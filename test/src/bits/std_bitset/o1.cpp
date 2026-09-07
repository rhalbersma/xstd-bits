//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/bitset/exhaustive.hpp>             // all_cardinality_sets, all_singleton_sets, all_valid, any_value, empty_set, full_set
#include <test/bitset/primitives.hpp>             // mem_set, mem_reset, mem_bit_not, mem_flip,
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <bitset>                                 // bitset
#include <cstdint>                                // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(O1)

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset<  0>
,         std::bitset<  1>
,         std::bitset< 64>
,        xstd::basic_bitset< 0, uint8_t>
,        xstd::basic_bitset< 1, uint8_t>
,        xstd::basic_bitset< 8, uint8_t>
,        xstd::basic_bitset< 9, uint8_t>
,        xstd::basic_bitset<16, uint8_t>
,        xstd::basic_bitset<17, uint8_t>
,        xstd::basic_bitset<24, uint8_t>
,        xstd::basic_bitset<24, uint16_t>
,        xstd::basic_bitset<24, uint32_t>
,        xstd::basic_bitset<24, uint64_t>
#ifdef TEST_HAS_UINT128
,        xstd::basic_bitset<24, xstd::uint128>
#endif
,        xstd::basic_dynamic_bitset<uint8_t>
,        xstd::basic_dynamic_bitset<uint64_t>
>;

using namespace test::bitset;

BOOST_AUTO_TEST_CASE_TEMPLATE(SetResetComplementAndFlipHoldOverEveryPosition, T, Types)
{
        on1::all_cardinality_sets<T>(mem_set());
        on1::all_cardinality_sets<T>(mem_set());
        on1::all_singleton_sets<T>(mem_set());
        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto& bs0) {
                        mem_set()(bs0, pos);
                });
                on0::empty_set<T>([&](auto& bs0) {
                        mem_set()(bs0, pos, true);
                });
                on0::full_set<T>([&](auto& bsN) {
                        mem_set()(bsN, pos, false);
                });
        });

        on1::all_cardinality_sets<T>(mem_reset());
        on1::all_singleton_sets<T>(mem_reset());
        on1::any_value<T>([](auto pos) {
                on0::full_set<T>([&](auto& bsN) {
                        mem_reset()(bsN, pos);
                });
        });

        on1::all_cardinality_sets<T>(mem_bit_not());
        on1::all_singleton_sets<T>(mem_bit_not());

        on1::all_cardinality_sets<T>(mem_flip());
        on1::all_singleton_sets<T>(mem_flip());
        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto& bs0) {
                        mem_flip()(bs0, pos);
                });
                on0::full_set<T>([&](auto& bsN) {
                        mem_flip()(bsN, pos);
                });
       });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AtHoldsOverEveryValidPosition, T, Types)
{
        on1::all_valid<T>([](auto pos) {
                on0::empty_set<T>([&](auto const& bs0) {
                        mem_at()(bs0, pos);
                });
                // empty_set<T>([&](auto& bs0) {
                //         mem_at()(bs0, pos);
                // });
                // empty_set<T>([&](auto& bs0) {
                //         mem_at()(bs0, pos, false);
                // });
                // empty_set<T>([&](auto& bs0) {
                //         mem_at()(bs0, pos, true);
                // });
                on0::full_set<T>([&](auto const& bsN) {
                        mem_at()(bsN, pos);
                });
                // full_set<T>([&](auto& bsN) {
                //         mem_at()(bsN, pos);
                // });
                // full_set<T>([&](auto& bsN) {
                //         mem_at()(bsN, pos, false);
                // });
                // full_set<T>([&](auto& bsN) {
                //         mem_at()(bsN, pos, true);
                // });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheObserversHoldOverEveryCardinalityAndSingleton, T, Types)
{
        on1::all_cardinality_sets<T>(mem_count());

        on1::all_cardinality_sets<T>(mem_size());
        on1::all_singleton_sets<T>(mem_size());

        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto const& bs0) {
                        mem_test()(bs0, pos);
                });
                on0::full_set<T>([&](auto const& bsN) {
                        mem_test()(bsN, pos);
                });
        });

        on1::all_cardinality_sets<T>(mem_all());
        on1::all_cardinality_sets<T>(mem_any());
        on1::all_cardinality_sets<T>(mem_none());

        // empty/full set vs. every singleton at matching N, so a dynamic_bitset pair is same-length and only cardinality differs.
        on1::all_singleton_sets<T>([](auto const& bs1) {
                on0::empty_set<T, limit_v<T, L1>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs1);
                });
                on0::full_set<T, limit_v<T, L1>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs1);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StreamingRoundTripsOverEveryCardinalityAndSingleton, T, Types)
{
        on1::all_cardinality_sets<T>(op_iostream());
        on1::all_singleton_sets<T>(op_iostream());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
