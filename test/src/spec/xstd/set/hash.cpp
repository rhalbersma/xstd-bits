//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_singleton_set_pairs, all_singleton_sets, empty_set_pair
#include <test/set/primitives.hpp>  // op_hash
#include <test/spec/random.hpp>     // all_set_pairs, all_sets
#include <test/spec/set.hpp>        // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Hash)

using namespace test;
using namespace test::set;

// std::set has no std::hash, so the models pass vacuously and the check is on every column that has one.
BOOST_AUTO_TEST_CASE_TEMPLATE(EqualSetsHashEqualOnAnEmptyPair, T, test::spec::set::every_width)
{
        on0::empty_set_pair<T>(op_hash());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EqualSetsHashEqualOverEverySingletonAndSingletonPair, T, test::spec::set::boundary_widths)
{
        on1::all_singleton_sets<T>(op_hash());
        on2::all_singleton_set_pairs<T>(op_hash());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EqualSetsHashEqualOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(op_hash());
        spec::random::all_set_pairs<T>(op_hash());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
