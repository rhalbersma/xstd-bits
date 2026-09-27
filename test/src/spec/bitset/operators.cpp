//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_cardinality_sets, all_singleton_set_pairs, all_singleton_sets, empty_set_pair
#include <test/bitset/primitives.hpp> // op_bit_and, op_bit_or, op_bit_xor, op_iostream, op_istream_failure
#include <test/spec/bitset.hpp>       // boundary_widths, byte_widths, every_width, random_widths
#include <test/spec/random.hpp>       // all_bitset_pairs, all_bitsets
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(Operators)

using namespace test::bitset;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitwiseOperatorsHoldOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(op_bit_and());
        on0::empty_set_pair<T>(op_bit_or());
        on0::empty_set_pair<T>(op_bit_xor());
}

// A read that stores nothing fails; boost::dynamic_bitset's own extraction is not checked.
BOOST_AUTO_TEST_CASE_TEMPLATE(ExtractionSetsFailbitWhenNothingIsStored, T, test::spec::bitset::every_width)
{
        op_istream_failure<T>()();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StreamingRoundTripsOverEveryCardinalityAndSingleton, T, test::spec::bitset::boundary_widths)
{
        on1::all_cardinality_sets<T>(op_iostream());
        on1::all_singleton_sets<T>(op_iostream());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitwiseOperatorsHoldOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(op_bit_and());
        on2::all_singleton_set_pairs<T>(op_bit_or());
        on2::all_singleton_set_pairs<T>(op_bit_xor());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitwiseOperatorsHoldOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>([](auto const& a, auto const& b) {
                op_bit_and()(a, b);
                op_bit_or()(a, b);
                op_bit_xor()(a, b);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(StreamingRoundTripsOverRandomBitsets, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitsets<T>(op_iostream());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
