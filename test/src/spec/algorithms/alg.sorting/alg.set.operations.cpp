//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/composable.hpp>  // includes, set_difference, set_intersection, set_symmetric_difference, set_union
#include <test/set/exhaustive.hpp>  // all_doubleton_set_pairs, all_singleton_set_pairs, empty_set_pair
#include <test/spec/random.hpp>     // all_set_pairs
#include <test/spec/set.hpp>        // boundary_widths, every_width, few_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Algorithms)
BOOST_AUTO_TEST_SUITE(AlgSorting)
BOOST_AUTO_TEST_SUITE(AlgSetOperations)

using namespace test;
using namespace test::set;

// Each member operator a set has is the algorithm over its keys; a set without the member is skipped.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsAreTheAlgorithmsOnAnEmptyPair, T, test::spec::set::every_width)
{
        on0::empty_set_pair<T>(composable::includes());
        on0::empty_set_pair<T>(composable::set_union());
        on0::empty_set_pair<T>(composable::set_intersection());
        on0::empty_set_pair<T>(composable::set_difference());
        on0::empty_set_pair<T>(composable::set_symmetric_difference());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsAreTheAlgorithmsOverEverySingletonPair, T, test::spec::set::boundary_widths)
{
        on2::all_singleton_set_pairs<T>(composable::includes());
        on2::all_singleton_set_pairs<T>(composable::set_union());
        on2::all_singleton_set_pairs<T>(composable::set_intersection());
        on2::all_singleton_set_pairs<T>(composable::set_difference());
        on2::all_singleton_set_pairs<T>(composable::set_symmetric_difference());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IncludesIsTheAlgorithmOverEveryDoubletonPair, T, test::spec::set::few_widths)
{
        on4::all_doubleton_set_pairs<T>(composable::includes());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSetOperationsAreTheAlgorithmsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_set_pairs<T>([](auto const& a, auto const& b) {
                composable::includes()(a, b);
                composable::set_union()(a, b);
                composable::set_intersection()(a, b);
                composable::set_difference()(a, b);
                composable::set_symmetric_difference()(a, b);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
