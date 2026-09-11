//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp> // all_cardinality_sets, all_singleton_arrays, all_singleton_ilists, all_singleton_sets,
                                        // all_valid, empty_set, full_set
#include <test/flat_set.hpp>            // TEST_HAS_FLAT_SET, is_flat_set
#include <test/set/primitives.hpp>      // constructor, mem_const_reference, mem_const_iterator, mem_front, mem_back,
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                       // from_range
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(O1)

using namespace test;
using namespace test::set;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::basic_bit_static_set<uint8_t, 0>
,       xstd::basic_bit_static_set<uint8_t, 1>
,       xstd::basic_bit_static_set<uint8_t, 8>
,       xstd::basic_bit_static_set<uint8_t, 9>
,       xstd::basic_bit_static_set<uint8_t, 16>
,       xstd::basic_bit_static_set<uint8_t, 17>
,       xstd::basic_bit_static_set<uint8_t, 24>
,       xstd::basic_bit_static_set<uint16_t, 24>
,       xstd::basic_bit_static_set<uint32_t, 24>
,       xstd::basic_bit_static_set<uint64_t, 24>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<xstd::uint128, 24>
#endif
,       xstd::basic_bit_set<uint8_t>
,       xstd::basic_bit_set<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsHoldOverEverySingleton, T, Types)
{
        on1::all_singleton_arrays<T>([](auto const& a1) {
                constructor<T>()(a1.begin(), a1.end());
                constructor<T>()(std::from_range, a1);
        });
        on1::all_singleton_ilists<T>([](auto ilist1) {
                constructor<T>()(std::from_range, ilist1);
                constructor<T>()(ilist1);
        });

        on1::all_cardinality_sets<T>(mem_const_reference());
        on1::all_singleton_sets<T>(mem_const_reference());

        on1::all_cardinality_sets<T>([](auto& is) {
                mem_const_iterator()(is);
        });
        on1::all_cardinality_sets<T>([](auto const& is) {
                mem_const_iterator()(is);
        });
        on1::all_singleton_sets<T>([](auto& is1) {
                mem_const_iterator()(is1);
        });
        on1::all_singleton_sets<T>([](auto const& is1) {
                mem_const_iterator()(is1);
        });

        on1::all_cardinality_sets<T>(mem_front());
        on1::all_singleton_sets<T>(mem_front());

        on1::all_cardinality_sets<T>(mem_back());
        on1::all_singleton_sets<T>(mem_back());

        on1::all_cardinality_sets<T>(mem_empty());
        on1::all_cardinality_sets<T>(mem_size());
        on1::all_cardinality_sets<T>(mem_max_size());

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
                        mem_emplace_hint()(isN, isN.end(), t);
                });
        });
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

        on1::all_valid<T>([](auto const& k) {
                on0::empty_set<T>([&](auto& is0) {
                        mem_erase()(is0, k);
                });
                on0::full_set<T>([&](auto& isN) {
                        mem_erase()(isN, k);
                });
        });

        // std:flat_set<std::size_t>::erase invalidates iterators
        if constexpr (not is_flat_set<T>) {
                on0::full_set<T>([](auto& isN) {
                        for (auto first = isN.begin(), last = isN.end(); first != last; /* expression inside loop */) {
                                mem_erase()(isN, first++);
                        }
                });
                on0::full_set<T>([](auto& isN) {
                        mem_erase()(isN, isN.begin(), isN.end());
                });
        }

        on1::all_cardinality_sets<T>(mem_clear());
        on1::all_singleton_sets<T>(mem_clear());

        on1::all_cardinality_sets<T>(op_compare_three_way());
        on1::all_singleton_sets<T>(op_compare_three_way());
        on1::all_singleton_sets<T>(op_hash());

        on1::all_cardinality_sets<T>([](auto& is) {
                fn_iterator()(is);
        });
        on1::all_cardinality_sets<T>([](auto const& is) {
                fn_iterator()(is);
        });

        on1::all_cardinality_sets<T>(fn_size());
        on1::all_cardinality_sets<T>(fn_ssize());
        on1::all_cardinality_sets<T>(fn_empty());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
