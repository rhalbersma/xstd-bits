//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/bitset/exhaustive.hpp>             // all_doubleton_set_pairs
#include <test/bitset/primitives.hpp>             // mem_compare_three_way, mem_is_subset_of, mem_is_proper_subset_of
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <xstd/bits/ext/xstd/bitset.hpp>          // bitset
#include <bitset>                                 // bitset
#include <cstdint>                                // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(O4)

using namespace test::bitset;

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset< 0>
,         std::bitset<17>
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
,        xstd::basic_dynamic_bitset<uint8_t>
,        xstd::basic_dynamic_bitset<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingAndSubsetTestsHoldOverEveryDoubletonPair, T, Types)
{
        on4::all_doubleton_set_pairs<T>(mem_compare_three_way());
        on4::all_doubleton_set_pairs<T>(mem_is_subset_of());
        on4::all_doubleton_set_pairs<T>(mem_is_proper_subset_of());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
