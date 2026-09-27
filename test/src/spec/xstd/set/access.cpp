//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_cardinality_sets, all_singleton_sets
#include <test/set/primitives.hpp>  // mem_back, mem_front
#include <test/spec/random.hpp>     // all_sets
#include <test/spec/set.hpp>        // boundary_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Access)

using namespace test;
using namespace test::set;

// front and back are [sequence.reqmts]'s, which std::set lacks, so the models pass vacuously.
BOOST_AUTO_TEST_CASE_TEMPLATE(FrontAndBackAreTheFirstAndLastKeyOverEveryCardinalityAndSingleton, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_front());
        on1::all_singleton_sets<T>(mem_front());

        on1::all_cardinality_sets<T>(mem_back());
        on1::all_singleton_sets<T>(mem_back());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FrontAndBackAreTheFirstAndLastKeyOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(mem_front());
        spec::random::all_sets<T>(mem_back());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
