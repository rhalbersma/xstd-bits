//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/bitset/exhaustive.hpp>             // all_singleton_sets, all_singleton_set_pairs, all_doubleton_sets, any_value, empty_set, full_set
#include <test/bitset/primitives.hpp>             // mem_bit_and_assign, mem_bit_or_assign, mem_bit_xor_assign, mem_bit_minus_assign,
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/basic_bitset.hpp>             // basic_bitset
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <xstd/bits/ext/xstd/bitset.hpp>          // bitset
#include <bitset>                                 // bitset
#include <cstdint>                                // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(O2)

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset<0>
,         std::bitset<8>
,        xstd::basic_bitset<std::bitset< 0>>
,        xstd::basic_bitset<std::bitset< 8>>
,        xstd::bitset< 0, uint8_t>
,        xstd::bitset< 8, uint8_t>
,        xstd::bitset< 9, uint8_t>
,        xstd::bitset<17, uint8_t>
,        xstd::bitset< 8, uint16_t>
,        xstd::bitset< 8, uint32_t>
,        xstd::bitset< 8, uint64_t>
#ifdef TEST_HAS_UINT128
,        xstd::bitset< 8, xstd::uint128>
#endif
,        xstd::basic_bitset<boost::dynamic_bitset<>>
,        xstd::dynamic_bitset<uint8_t>
,        xstd::dynamic_bitset<uint64_t>
>;

using namespace test::bitset;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheCompoundAssignmentsAndShiftsHoldOverEverySingletonPair, T, Types)
{
        on2::all_singleton_set_pairs<T>(mem_bit_and_assign());
        on2::all_singleton_set_pairs<T>(mem_bit_or_assign());
        on2::all_singleton_set_pairs<T>(mem_bit_xor_assign());
        on2::all_singleton_set_pairs<T>(mem_bit_minus_assign());

        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto& bs1) {
                        mem_shift_left_assign()(bs1, pos);
                });
        });
        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto& bs1) {
                        mem_shift_right_assign()(bs1, pos);
                });
        });

        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        mem_shift_left()(bs1, pos);
                });
        });
        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        mem_shift_right()(bs1, pos);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheComparisonsHoldOverEverySingletonPair, T, Types)
{
        on2::all_singleton_set_pairs<T>(mem_equal_to());
        on2::all_singleton_set_pairs<T>(mem_compare_three_way());
        on2::all_singleton_set_pairs<T>(mem_is_subset_of());
        on2::all_singleton_set_pairs<T>(mem_is_proper_subset_of());
        on2::all_singleton_set_pairs<T>(mem_intersects());

        // empty/full set vs. every doubleton at matching N -- see o1.cpp for why N is pinned.
        on2::all_doubleton_sets<T, limit_v<T, L4>>([](auto const& bs2) {
                on0::empty_set<T, limit_v<T, L4>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs2);
                });
                on0::full_set<T, limit_v<T, L4>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs2);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitwiseOperatorsHoldOverEverySingletonPair, T, Types)
{
        on2::all_singleton_set_pairs<T>(op_bit_and());
        on2::all_singleton_set_pairs<T>(op_bit_or());
        on2::all_singleton_set_pairs<T>(op_bit_xor());
        on2::all_singleton_set_pairs<T>(op_bit_minus());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
