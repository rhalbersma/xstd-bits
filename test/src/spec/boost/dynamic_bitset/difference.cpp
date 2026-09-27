//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_singleton_set_pairs, empty_set_pair
#include <test/bitset/primitives.hpp> // mem_bit_minus_assign, op_bit_minus
#include <test/spec/bitset.hpp>       // byte_widths, every_width, random_widths
#include <test/spec/random.hpp>       // all_bitset_pairs
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Difference)

using namespace test::bitset;

// std::bitset has no set difference, so it passes vacuously and the check is on every candidate that has one.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheDifferenceHoldsOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_bit_minus_assign());
        on0::empty_set_pair<T>(op_bit_minus());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheDifferenceHoldsOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_bit_minus_assign());
        on2::all_singleton_set_pairs<T>(op_bit_minus());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheDifferenceHoldsOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>([](auto const& a, auto const& b) {
                op_bit_minus()(a, b);
                auto x = a;
                mem_bit_minus_assign()(x, b);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
