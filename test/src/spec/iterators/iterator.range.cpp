//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/exhaustive.hpp>  // all_cardinality_sets
#include <test/set/primitives.hpp>  // fn_empty, fn_iterator, fn_size, fn_ssize
#include <test/spec/random.hpp>     // all_sets
#include <test/spec/set.hpp>        // boundary_widths, random_widths
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <utility>                  // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Iterators)
BOOST_AUTO_TEST_SUITE(IteratorRange)

using namespace test;
using namespace test::set;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheRangeAccessFunctionsAgreeWithTheMembersOverEveryCardinality, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>([](auto& is) {
                fn_iterator()(is);
        });
        on1::all_cardinality_sets<T>([](auto const& is) {
                fn_iterator()(is);
        });

        on1::all_cardinality_sets<T>(fn_size());
        on1::all_cardinality_sets<T>(fn_ssize());
        on1::all_cardinality_sets<T>(fn_empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheRangeAccessFunctionsAgreeWithTheMembersOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>([](auto& is) {
                fn_iterator()(is);
                fn_iterator()(std::as_const(is));
                fn_size()(is);
                fn_ssize()(is);
                fn_empty()(is);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
