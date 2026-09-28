//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/composable.hpp>  // includes
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, pairs_with_doubletons
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Algorithms)
BOOST_AUTO_TEST_SUITE(AlgSorting)
BOOST_AUTO_TEST_SUITE(AlgSetOperations)
BOOST_AUTO_TEST_SUITE(Includes)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// [includes]/1-4: bool includes(first1, last1, first2, last2)
BOOST_AUTO_TEST_CASE(Includes)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // Each member operator a set has is the algorithm over its keys; a set without the member is skipped.
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::includes()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
