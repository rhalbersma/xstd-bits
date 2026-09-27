//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // L1, L3, L4, all_doubleton_set_pairs, all_doubleton_sets, all_singleton_set_pairs, all_singleton_sets, all_triplet_sets, empty_set, empty_set_pair, full_set, limit_v
#include <test/bitset/primitives.hpp> // mem_compare_three_way
#include <test/spec/bitset.hpp>       // boundary_widths, byte_widths, every_width, few_widths, random_widths
#include <test/spec/random.hpp>       // all_bitset_pairs
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Ordering)

using namespace test::bitset;

// boost::dynamic_bitset's operator< orders the bit strings, std::bitset has no ordering and passes vacuously.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_compare_three_way());
}

// The empty and full sets against every singleton at a matching width, so a run-time pair differs only in its bits.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldBetweenEverySingletonAndTheEmptyAndFullSets, T, test::spec::bitset::boundary_widths)
{
        on1::all_singleton_sets<T>([](auto const& bs1) {
                on0::empty_set<T, limit_v<T, L1>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs1);
                });
                on0::full_set<T, limit_v<T, L1>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs1);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldOverEverySingletonPairAndAgainstEveryDoubleton, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_compare_three_way());

        on2::all_doubleton_sets<T, limit_v<T, L4>>([](auto const& bs2) {
                on0::empty_set<T, limit_v<T, L4>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs2);
                });
                on0::full_set<T, limit_v<T, L4>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs2);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldAgainstEveryTripletAndBetweenSingletonsAndDoubletons, T, test::spec::bitset::few_widths)
{
        on3::all_triplet_sets<T>([](auto const& bs3) {
                on0::empty_set<T, limit_v<T, L3>>([&](auto const& bs0) {
                        mem_compare_three_way()(bs0, bs3);
                });
                on0::full_set<T, limit_v<T, L3>>([&](auto const& bsN) {
                        mem_compare_three_way()(bsN, bs3);
                });
        });

        on2::all_doubleton_sets<T, limit_v<T, L3>>([](auto const& bs2) {
                on1::all_singleton_sets<T, limit_v<T, L3>>([&](auto const& bs1) {
                        mem_compare_three_way()(bs1, bs2);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldOverEveryDoubletonPair, T, test::spec::bitset::few_widths)
{
        on4::all_doubleton_set_pairs<T>(mem_compare_three_way());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingsHoldOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>(mem_compare_three_way());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
