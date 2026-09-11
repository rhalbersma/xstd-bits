//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/set/exhaustive.hpp>      // all_singleton_set_triples
#include <test/set/primitives.hpp>      // op_less
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <cstddef>                      // size_t
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(O3)

using namespace test;
using namespace test::set;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::basic_bit_static_set< 0, uint8_t>
,       xstd::basic_bit_static_set< 8, uint8_t>
,       xstd::basic_bit_static_set< 9, uint8_t>
,       xstd::basic_bit_static_set<17, uint8_t>
,       xstd::basic_bit_static_set<17, uint16_t>
,       xstd::basic_bit_static_set<17, uint32_t>
,       xstd::basic_bit_static_set<17, uint64_t>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<17, xstd::uint128>
#endif
,       xstd::basic_bit_set<uint8_t>
,       xstd::basic_bit_set<uint64_t>
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(LessIsTransitiveOverEverySingletonTriple, T, Types)
{
        on3::all_singleton_set_triples<T>(op_less());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
