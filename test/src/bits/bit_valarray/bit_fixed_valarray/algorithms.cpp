//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                          // graded_extents, wide_extents
#include <test/valarray.hpp>                             // check_*
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // basic_bit_fixed_valarray
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <tuple>                                         // tuple_cat
#include <utility>                                       // declval

BOOST_AUTO_TEST_SUITE(BitFixedValarray)

// Every Block model within one block, the narrow ones across boundaries, and the widest across one too.
using Types = decltype(std::tuple_cat(
        std::declval<test::graded_extents<xstd::basic_bit_fixed_valarray>>(),
        std::declval<test::wide_extents<xstd::basic_bit_fixed_valarray>>()
));

BOOST_AUTO_TEST_CASE_TEMPLATE(BeginAndEndMakeItARandomAccessRange, V, Types)
{
        test::valarray::check_range<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitAlgorithmsReadAndWriteIt, V, Types)
{
        test::valarray::check_algorithms<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BitConvertKeepsPositionsAndASpanViewsIt, V, Types)
{
        test::valarray::check_interop<V>();
}

BOOST_AUTO_TEST_SUITE_END()
