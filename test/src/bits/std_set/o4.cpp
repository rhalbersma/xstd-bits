//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/set/composable.hpp>      // includes
#include <test/set/exhaustive.hpp>      // all_doubleton_set_pairs
#include <test/set/primitives.hpp>      // op_compare_three_way
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(O4)

using namespace test;
using namespace test::set;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::basic_bit_static_set<uint8_t, 0>
,       xstd::basic_bit_static_set<uint8_t, 8>
,       xstd::basic_bit_static_set<uint8_t, 9>
,       xstd::basic_bit_static_set<uint8_t, 17>
,       xstd::basic_bit_static_set<uint16_t, 17>
,       xstd::basic_bit_static_set<uint32_t, 17>
,       xstd::basic_bit_static_set<uint64_t, 17>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<xstd::uint128, 17>
#endif
,       xstd::basic_bit_set<uint8_t>
,       xstd::basic_bit_set<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareThreeWayHoldsOverEveryDoubletonPair, T, Types)
{
        on4::all_doubleton_set_pairs<T>(op_compare_three_way());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IncludesHoldsOverEveryDoubletonPair, T, Types)
{
        on4::all_doubleton_set_pairs<T>(composable::includes());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
