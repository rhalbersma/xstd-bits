//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_cardinality_sets, all_doubleton_set_pairs, all_singleton_set_pairs, all_singleton_set_triples, all_singleton_sets, empty_set_pair
#include <test/set/primitives.hpp>  // op_compare_three_way, op_greater, op_greater_equal, op_less, op_less_equal
#include <test/spec/random.hpp>     // all_set_pairs, all_set_triples, all_sets
#include <test/spec/set.hpp>        // boundary_widths, every_width, few_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerOptReqmts)

using namespace test;
using namespace test::set;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingHoldsOnAnEmptyPair, T, test::spec::set::every_width)
{
        on0::empty_set_pair<T>(op_compare_three_way());
        on0::empty_set_pair<T>(op_less());
        on0::empty_set_pair<T>(op_greater());
        on0::empty_set_pair<T>(op_less_equal());
        on0::empty_set_pair<T>(op_greater_equal());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareThreeWayIsReflexiveOverEveryCardinalityAndSingleton, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(op_compare_three_way());
        on1::all_singleton_sets<T>(op_compare_three_way());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingHoldsOverEverySingletonPair, T, test::spec::set::boundary_widths)
{
        on2::all_singleton_set_pairs<T>(op_compare_three_way());
        on2::all_singleton_set_pairs<T>(op_less());
        on2::all_singleton_set_pairs<T>(op_greater());
        on2::all_singleton_set_pairs<T>(op_less_equal());
        on2::all_singleton_set_pairs<T>(op_greater_equal());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(LessIsTransitiveOverEverySingletonTriple, T, test::spec::set::few_widths)
{
        on3::all_singleton_set_triples<T>(op_less());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompareThreeWayHoldsOverEveryDoubletonPair, T, test::spec::set::few_widths)
{
        on4::all_doubleton_set_pairs<T>(op_compare_three_way());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheOrderingHoldsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(op_compare_three_way());
        spec::random::all_sets<T>(op_less());
        spec::random::all_set_pairs<T>([](auto const& a, auto const& b) {
                op_compare_three_way()(a, b);
                op_less()(a, b);
                op_greater()(a, b);
                op_less_equal()(a, b);
                op_greater_equal()(a, b);
        });
        spec::random::all_set_triples<T>(op_less());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
