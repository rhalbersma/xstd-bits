//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/bitset/exhaustive.hpp>             // empty_set_pair
#include <test/bitset/primitives.hpp>             // constructor,
#include <test/uint128.hpp>                       // TEST_HAS_UINT128, uint128
#include <xstd/bits/bitset.hpp>                   // bitset
#include <xstd/bits/dynamic_bitset.hpp>           // dynamic_bitset
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // dynamic_bitset
#include <xstd/bits/ext/std/bitset.hpp>           // bitset
#include <bitset>                                 // bitset
#include <cstdint>                                // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                                  // tuple

BOOST_AUTO_TEST_SUITE(StdBitset)
BOOST_AUTO_TEST_SUITE(O0)

using Types = std::tuple
<       boost::dynamic_bitset<>
,         std::bitset<  0>
,         std::bitset<  1>
,         std::bitset< 31>
,         std::bitset< 32>
,         std::bitset< 33>
,         std::bitset< 63>
,         std::bitset< 64>
,         std::bitset< 65>
,        xstd::basic_bitset<  0, uint8_t>
,        xstd::basic_bitset<  1, uint8_t>
,        xstd::basic_bitset<  7, uint8_t>
,        xstd::basic_bitset<  8, uint8_t>
,        xstd::basic_bitset<  9, uint8_t>
,        xstd::basic_bitset< 15, uint8_t>
,        xstd::basic_bitset< 16, uint8_t>
,        xstd::basic_bitset< 17, uint8_t>
,        xstd::basic_bitset< 24, uint8_t>
,        xstd::basic_bitset<  0, uint16_t>
,        xstd::basic_bitset<  1, uint16_t>
,        xstd::basic_bitset< 15, uint16_t>
,        xstd::basic_bitset< 16, uint16_t>
,        xstd::basic_bitset< 17, uint16_t>
,        xstd::basic_bitset< 31, uint16_t>
,        xstd::basic_bitset< 32, uint16_t>
,        xstd::basic_bitset< 33, uint16_t>
,        xstd::basic_bitset< 48, uint16_t>
,        xstd::basic_bitset<  0, uint32_t>
,        xstd::basic_bitset<  1, uint32_t>
,        xstd::basic_bitset< 31, uint32_t>
,        xstd::basic_bitset< 32, uint32_t>
,        xstd::basic_bitset< 33, uint32_t>
,        xstd::basic_bitset< 63, uint32_t>
,        xstd::basic_bitset< 64, uint32_t>
,        xstd::basic_bitset< 65, uint32_t>
,        xstd::basic_bitset<  0, uint64_t>
,        xstd::basic_bitset<  1, uint64_t>
,        xstd::basic_bitset< 63, uint64_t>
,        xstd::basic_bitset< 64, uint64_t>
,        xstd::basic_bitset< 65, uint64_t>
#ifdef TEST_HAS_UINT128
,        xstd::basic_bitset<  0, xstd::uint128>
,        xstd::basic_bitset<  1, xstd::uint128>
,        xstd::basic_bitset<127, xstd::uint128>
,        xstd::basic_bitset<128, xstd::uint128>
,        xstd::basic_bitset<129, xstd::uint128>
#endif
,        xstd::basic_dynamic_bitset<uint8_t>
,        xstd::basic_dynamic_bitset<uint64_t>
>;

using namespace test::bitset;

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstructionYieldsAnEmptySet, T, Types)
{
        constructor<T>()();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheCompoundBitwiseAssignmentsHoldOnAnEmptyPair, T, Types)
{
        on0::empty_set_pair<T>(mem_bit_and_assign());
        on0::empty_set_pair<T>(mem_bit_or_assign());
        on0::empty_set_pair<T>(mem_bit_xor_assign());
        on0::empty_set_pair<T>(mem_bit_minus_assign());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheComparisonsAndSetPredicatesHoldOnAnEmptyPair, T, Types)
{
        on0::empty_set_pair<T>(mem_equal_to());
        on0::empty_set_pair<T>(mem_compare_three_way());
        on0::empty_set_pair<T>(mem_is_subset_of());
        on0::empty_set_pair<T>(mem_is_proper_subset_of());
        on0::empty_set_pair<T>(mem_is_proper_subset_of_edges());
        on0::empty_set_pair<T>(mem_intersects());
        on0::empty_set_pair<T>(op_hash());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitwiseOperatorsHoldOnAnEmptyPairAndExtractionRespectsFailbit, T, Types)
{
        on0::empty_set_pair<T>(op_bit_and());
        on0::empty_set_pair<T>(op_bit_or());
        on0::empty_set_pair<T>(op_bit_xor());
        on0::empty_set_pair<T>(op_bit_minus());
        op_istream_failure<T>()();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
