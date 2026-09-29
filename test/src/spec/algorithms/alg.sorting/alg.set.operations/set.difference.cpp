//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // fn_set_difference, fn_ranges_set_difference
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, pairs
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Algorithms)
BOOST_AUTO_TEST_SUITE(AlgSorting)
BOOST_AUTO_TEST_SUITE(AlgSetOperations)
BOOST_AUTO_TEST_SUITE(SetDifference)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// [set.difference]/1-5: OutputIterator set_difference(first1, last1, first2, last2, result, comp)
BOOST_AUTO_TEST_CASE(SetDifference)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        fn_set_difference()(a, b);
                }
        });
}

// [set.difference]/1,3-5: ranges::set_difference(r1, r2, result, comp, proj1, proj2)
BOOST_AUTO_TEST_CASE(RangesSetDifference)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        fn_ranges_set_difference()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
