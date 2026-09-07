//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/set/composable.hpp>      // includes, set_difference, set_intersection, set_symmetric_difference, set_union
#include <test/set/exhaustive.hpp>      // empty_set_pair
#include <test/set/primitives.hpp>      // constructor mem_swap,fn_swap, op_equal, op_not_equal_to,
                                        // op_compare_three_way op_less, op_greater, op_less_equal, op_greater_equal,
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(O0)

using namespace test;
using namespace test::set;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::basic_bit_static_set<  0, uint8_t>
,       xstd::basic_bit_static_set<  1, uint8_t>
,       xstd::basic_bit_static_set<  7, uint8_t>
,       xstd::basic_bit_static_set<  8, uint8_t>
,       xstd::basic_bit_static_set<  9, uint8_t>
,       xstd::basic_bit_static_set< 15, uint8_t>
,       xstd::basic_bit_static_set< 16, uint8_t>
,       xstd::basic_bit_static_set< 17, uint8_t>
,       xstd::basic_bit_static_set< 24, uint8_t>
,       xstd::basic_bit_static_set<  0, uint16_t>
,       xstd::basic_bit_static_set<  1, uint16_t>
,       xstd::basic_bit_static_set< 15, uint16_t>
,       xstd::basic_bit_static_set< 16, uint16_t>
,       xstd::basic_bit_static_set< 17, uint16_t>
,       xstd::basic_bit_static_set< 31, uint16_t>
,       xstd::basic_bit_static_set< 32, uint16_t>
,       xstd::basic_bit_static_set< 33, uint16_t>
,       xstd::basic_bit_static_set< 48, uint16_t>
,       xstd::basic_bit_static_set<  0, uint32_t>
,       xstd::basic_bit_static_set<  1, uint32_t>
,       xstd::basic_bit_static_set< 31, uint32_t>
,       xstd::basic_bit_static_set< 32, uint32_t>
,       xstd::basic_bit_static_set< 33, uint32_t>
,       xstd::basic_bit_static_set< 63, uint32_t>
,       xstd::basic_bit_static_set< 64, uint32_t>
,       xstd::basic_bit_static_set< 65, uint32_t>
,       xstd::basic_bit_static_set<  0, uint64_t>
,       xstd::basic_bit_static_set<  1, uint64_t>
,       xstd::basic_bit_static_set< 63, uint64_t>
,       xstd::basic_bit_static_set< 64, uint64_t>
,       xstd::basic_bit_static_set< 65, uint64_t>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<  0, xstd::uint128>
,       xstd::basic_bit_static_set<  1, xstd::uint128>
,       xstd::basic_bit_static_set<127, xstd::uint128>
,       xstd::basic_bit_static_set<128, xstd::uint128>
,       xstd::basic_bit_static_set<129, xstd::uint128>
#endif
,       xstd::basic_bit_set<uint8_t>
,       xstd::basic_bit_set<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsHoldOnAnEmptyPair, T, Types)
{
        [[maybe_unused]] auto const _ = nested_types<T>();
        constructor<T>()();

        on0::empty_set_pair<T>(mem_swap());
        on0::empty_set_pair<T>(fn_swap());

        on0::empty_set_pair<T>(op_equal_to());
        on0::empty_set_pair<T>(op_not_equal_to());
        on0::empty_set_pair<T>(op_hash());

        on0::empty_set_pair<T>(op_compare_three_way());
        on0::empty_set_pair<T>(op_less());
        on0::empty_set_pair<T>(op_greater());
        on0::empty_set_pair<T>(op_less_equal());
        on0::empty_set_pair<T>(op_greater_equal());

        on0::empty_set_pair<T>(composable::includes());
        on0::empty_set_pair<T>(composable::set_union());
        on0::empty_set_pair<T>(composable::set_intersection());
        on0::empty_set_pair<T>(composable::set_difference());
        on0::empty_set_pair<T>(composable::set_symmetric_difference());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
