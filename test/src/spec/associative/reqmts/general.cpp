//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>        // is_flat_set
#include <test/set/exhaustive.hpp>  // all_cardinality_sets, all_doubleton_arrays, all_doubleton_ilists, all_doubleton_sets, all_singleton_arrays, all_singleton_ilists, all_singleton_sets, all_valid, empty_set, full_set
#include <test/set/primitives.hpp>  // mem_clear, mem_contains, mem_count, mem_emplace, mem_emplace_hint, mem_equal_range, mem_erase, mem_find, mem_insert, mem_lower_bound, mem_upper_bound
#include <test/spec/random.hpp>     // all_set_key_pairs, all_set_pairs, all_sets
#include <test/spec/set.hpp>        // boundary_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <utility>                  // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Reqmts)
BOOST_AUTO_TEST_SUITE(General)

using namespace test;
using namespace test::set;

BOOST_AUTO_TEST_CASE_TEMPLATE(EveryKeyInsertsIntoTheEmptyAndTheFullSet, T, test::spec::set::boundary_widths)
{
        on1::all_valid<T>([](auto const& t) {
                on0::empty_set<T>([=](auto& is0) {
                        mem_emplace()(is0, t);
                });
                on0::full_set<T>([=](auto& isN) {
                        mem_emplace()(isN, t);
                });
                on0::empty_set<T>([=](auto& is0) {
                        mem_insert()(is0, t);
                });
                on0::full_set<T>([=](auto& isN) {
                        mem_insert()(isN, t);
                });
        });
        on1::all_valid<T>([](auto const& t) {
                on0::empty_set<T>([=](auto& is0) {
                        mem_emplace_hint()(is0, is0.end(), t);
                });
                on0::full_set<T>([=](auto& isN) {
                        mem_emplace_hint()(isN, isN.end(), t);
                });
                on0::empty_set<T>([=](auto& is0) {
                        mem_insert()(is0, is0.end(), t);
                });
                on0::full_set<T>([=](auto& isN) {
                        mem_insert()(isN, isN.end(), t);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EverySingletonAndDoubletonRangeInsertsIntoTheEmptyAndTheFullSet, T, test::spec::set::boundary_widths)
{
        on1::all_singleton_arrays<T>([](auto const& a1) {
                on0::empty_set<T>([&](auto& is0) {
                        mem_insert()(is0, a1.begin(), a1.end());
                });
                on0::full_set<T>([&](auto& isN) {
                        mem_insert()(isN, a1.begin(), a1.end());
                });
        });
        on1::all_singleton_ilists<T>([](auto ilist1) {
                on0::empty_set<T>([&](auto& is0) {
                        mem_insert()(is0, ilist1);
                });
                on0::full_set<T>([&](auto& isN) {
                        mem_insert()(isN, ilist1);
                });
        });
        on2::all_doubleton_arrays<T>([](auto const& a2) {
                on0::empty_set<T>([&](auto& is0) {
                        mem_insert()(is0, a2.begin(), a2.end());
                });
                on0::full_set<T>([&](auto& isN) {
                        mem_insert()(isN, a2.begin(), a2.end());
                });
        });
        on2::all_doubleton_ilists<T>([](auto ilist2) {
                on0::empty_set<T>([=](auto& is0) {
                        mem_insert()(is0, ilist2);
                });
                on0::full_set<T>([=](auto& isN) {
                        mem_insert()(isN, ilist2);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EveryKeyAndEveryIteratorErases, T, test::spec::set::boundary_widths)
{
        on1::all_valid<T>([](auto const& k) {
                on0::empty_set<T>([&](auto& is0) {
                        mem_erase()(is0, k);
                });
                on0::full_set<T>([&](auto& isN) {
                        mem_erase()(isN, k);
                });
        });

        // std::flat_set<std::size_t>::erase invalidates iterators
        if constexpr (not is_flat_set<T>) {
                on0::full_set<T>([](auto& isN) {
                        for (auto first = isN.begin(), last = isN.end(); first != last; /* expression inside loop */) {
                                mem_erase()(isN, first++);
                        }
                });
                on0::full_set<T>([](auto& isN) {
                        mem_erase()(isN, isN.begin(), isN.end());
                });
                on2::all_doubleton_sets<T>([](auto& is2) {
                        mem_erase()(is2, is2.begin(), is2.end());
                });
        }

        on1::all_cardinality_sets<T>(mem_clear());
        on1::all_singleton_sets<T>(mem_clear());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EveryKeyIsLookedUpInEverySingleton, T, test::spec::set::boundary_widths)
{
        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto& is1) {
                        mem_find()(is1, x);
                        mem_lower_bound()(is1, x);
                        mem_upper_bound()(is1, x);
                        mem_equal_range()(is1, x);
                });
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_find()(is1, x);
                        mem_count()(is1, x);
                        mem_contains()(is1, x);
                        mem_lower_bound()(is1, x);
                        mem_upper_bound()(is1, x);
                        mem_equal_range()(is1, x);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(RandomKeysInsertEraseAndAreLookedUp, T, test::spec::set::random_widths)
{
        spec::random::all_set_key_pairs<T>([](auto& a, auto const& k) {
                mem_find()(a, k);
                mem_find()(std::as_const(a), k);
                mem_count()(a, k);
                mem_contains()(a, k);
                mem_lower_bound()(a, k);
                mem_lower_bound()(std::as_const(a), k);
                mem_upper_bound()(a, k);
                mem_upper_bound()(std::as_const(a), k);
                mem_equal_range()(a, k);
                mem_equal_range()(std::as_const(a), k);
                auto b = a;
                mem_insert()(b, k);
                mem_erase()(b, k);
                mem_emplace()(b, k);
                mem_emplace_hint()(b, b.end(), k);
                mem_insert()(b, b.end(), k);
                if constexpr (not is_flat_set<T>) {
                        mem_erase()(b, b.find(k));
                }
        });
        spec::random::all_set_pairs<T>([](auto& a, auto const& b) {
                mem_insert()(a, b.begin(), b.end());
        });
        spec::random::all_sets<T>(mem_clear());
        if constexpr (not is_flat_set<T>) {
                spec::random::all_sets<T>([](auto& a) {
                        mem_erase()(a, a.begin(), a.end());
                });
        }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
