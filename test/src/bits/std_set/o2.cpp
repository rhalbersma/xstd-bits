//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/composable.hpp> // includes, set_difference, set_intersection, set_symmetric_difference, set_union,
                                        // decrement, increment
#include <test/set/exhaustive.hpp> // all_doubleton_arrays, all_doubleton_ilists, all_doubleton_sets,
                                        // all_singleton_sets, all_singleton_set_pairs, all_valid
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/flat_set.hpp>            // TEST_HAS_FLAT_SET, is_flat_set
#include <test/set/primitives.hpp>      // constructor, op_assign, mem_insert, mem_erase, mem_swap, mem_find, mem_count,
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                       // from_range
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(O2)

using namespace test;
using namespace test::set;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::bit_static_set< 0, uint8_t>
,       xstd::bit_static_set< 1, uint8_t>
,       xstd::bit_static_set< 8, uint8_t>
,       xstd::bit_static_set< 9, uint8_t>
,       xstd::bit_static_set<16, uint8_t>
,       xstd::bit_static_set<17, uint8_t>
,       xstd::bit_static_set<24, uint8_t>
,       xstd::bit_static_set<24, uint16_t>
,       xstd::bit_static_set<24, uint32_t>
,       xstd::bit_static_set<24, uint64_t>
#ifdef TEST_HAS_UINT128
,       xstd::bit_static_set<24, xstd::uint128>
#endif
,       xstd::bit_set<uint8_t>
,       xstd::bit_set<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsHoldOverEveryDoubletonAndSingletonPair, T, Types)
{
        on2::all_doubleton_arrays<T>([](auto const& a2) {
                constructor<T>()(a2.begin(), a2.end());
                constructor<T>()(std::from_range, a2);
        });
        on2::all_doubleton_ilists<T>([](auto ilist2) {
                constructor<T>()(std::from_range, ilist2);
                constructor<T>()(ilist2);
        });

        on1::all_singleton_sets<T>([](auto& is1) {
                on1::all_singleton_ilists<T>([&](auto ilist1) {
                        op_assign()(is1, ilist1);
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

        // std::flat_set<std::size_t>::erase invalidates iterators
        if constexpr (not is_flat_set<T>) {
                on2::all_doubleton_sets<T>([](auto& is2) {
                        mem_erase()(is2, is2.begin(), is2.end());
                });
        }

        on2::all_singleton_set_pairs<T>(op_equal_to());
        on2::all_singleton_set_pairs<T>(mem_swap());
        on2::all_singleton_set_pairs<T>(fn_swap());

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto& is1) {
                        mem_find()(is1, x);
                });
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_find()(is1, x);
                });
        });

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_count()(is1, x);
                });
        });

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_contains()(is1, x);
                });
        });

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto& is1) {
                        mem_lower_bound()(is1, x);
                });
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_lower_bound()(is1, x);
                });
        });

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto& is1) {
                        mem_upper_bound()(is1, x);
                });
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_upper_bound()(is1, x);
                });
        });

        on1::all_valid<T>([](auto const& x) {
                on1::all_singleton_sets<T>([&](auto& is1) {
                        mem_equal_range()(is1, x);
                });
                on1::all_singleton_sets<T>([&](auto const& is1) {
                        mem_equal_range()(is1, x);
                });
        });

        on2::all_singleton_set_pairs<T>(op_not_equal_to());
        on2::all_singleton_set_pairs<T>(op_hash());

        on2::all_singleton_set_pairs<T>(op_compare_three_way());
        on2::all_singleton_set_pairs<T>(op_less());
        on2::all_singleton_set_pairs<T>(op_greater());
        on2::all_singleton_set_pairs<T>(op_less_equal());
        on2::all_singleton_set_pairs<T>(op_greater_equal());

        on2::all_singleton_set_pairs<T>(composable::includes());
        on2::all_singleton_set_pairs<T>(composable::set_union());
        on2::all_singleton_set_pairs<T>(composable::set_intersection());
        on2::all_singleton_set_pairs<T>(composable::set_difference());
        on2::all_singleton_set_pairs<T>(composable::set_symmetric_difference());

        on1::all_valid<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        composable::increment_modulo()(bs1, static_cast<std::size_t>(pos));
                });
        });
        on1::all_valid<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        composable::decrement_modulo()(bs1, static_cast<std::size_t>(pos));
                });
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
