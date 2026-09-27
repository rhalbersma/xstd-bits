//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_cardinality_sets, all_doubleton_arrays, all_doubleton_ilists, all_singleton_arrays, all_singleton_ilists, all_singleton_set_pairs, all_singleton_sets
#include <test/set/primitives.hpp>  // constructor, op_assign
#include <test/spec/random.hpp>     // all_key_vectors
#include <test/spec/set.hpp>        // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                   // from_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test;
using namespace test::set;

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstructionYieldsAnEmptySet, T, test::spec::set::every_width)
{
        constructor<T>()();
}

// std::less has no state, so a comparator argument is accepted and changes nothing.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheComparatorArgumentsAreAcceptedAsStdSetsAre, T, test::spec::set::boundary_widths)
{
        auto const comp = typename T::key_compare();
        BOOST_CHECK(T(comp).empty());
        on1::all_singleton_ilists<T>([&](auto ilist1) {
                BOOST_CHECK(T(ilist1, comp) == T(ilist1));
                BOOST_CHECK(T(ilist1.begin(), ilist1.end(), comp) == T(ilist1));
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ACopyOfEveryCardinalityIsEqual, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>([](auto const& is) {
                constructor<T>()(is);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructionFromEverySingletonIsInsertion, T, test::spec::set::boundary_widths)
{
        on1::all_singleton_arrays<T>([](auto const& a1) {
                constructor<T>()(a1.begin(), a1.end());
                constructor<T>()(std::from_range, a1);
        });
        on1::all_singleton_ilists<T>([](auto ilist1) {
                constructor<T>()(std::from_range, ilist1);
                constructor<T>()(ilist1);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructionFromEveryDoubletonIsInsertion, T, test::spec::set::boundary_widths)
{
        on2::all_doubleton_arrays<T>([](auto const& a2) {
                constructor<T>()(a2.begin(), a2.end());
                constructor<T>()(std::from_range, a2);
        });
        on2::all_doubleton_ilists<T>([](auto ilist2) {
                constructor<T>()(std::from_range, ilist2);
                constructor<T>()(ilist2);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AssignmentFromEverySingletonIsConstruction, T, test::spec::set::boundary_widths)
{
        on1::all_singleton_sets<T>([](auto& is1) {
                on1::all_singleton_ilists<T>([&](auto ilist1) {
                        op_assign()(is1, ilist1);
                });
        });
        on2::all_singleton_set_pairs<T>(op_assign());
}

// Keys in the order they were drawn, so the input is unsorted as often as not.
BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructionFromRandomKeysIsInsertion, T, test::spec::set::random_widths)
{
        spec::random::all_key_vectors<T>([](auto const& v, auto) {
                constructor<T>()(v.begin(), v.end());
                constructor<T>()(std::from_range, v);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
