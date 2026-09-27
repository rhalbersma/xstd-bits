//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_singleton_set_pairs, empty_set_pair
#include <test/bitset/primitives.hpp> // mem_intersects
#include <test/spec/bitset.hpp>       // byte_widths, every_width, random_widths
#include <test/spec/random.hpp>       // all_bitset_pairs
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Intersects)

using namespace test::bitset;

// std::bitset has no intersects, so it is answered position by position, which is what the member is checked against.
BOOST_AUTO_TEST_CASE_TEMPLATE(IntersectsHoldsOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_intersects());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IntersectsHoldsOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_intersects());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IntersectsHoldsOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>(mem_intersects());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
