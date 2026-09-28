//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/composable.hpp>  // decrement_modulo, increment_modulo, increment_within_capacity
#include <test/set/exhaustive.hpp>  // static_capacity
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, keyed_sets_with_singletons
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Shift)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// xstd set: constexpr X operator<<(const X& lhs, size_t n);
BOOST_AUTO_TEST_CASE(ShiftLeft)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // A left shift keeps every key, and under a capacity throws where the highest one would not fit.
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        if constexpr (not static_capacity<T>) {
                                composable::increment_modulo()(a, k);
                        } else {
                                composable::increment_within_capacity()(a, k);
                        }
                }
        });
}

// xstd set: constexpr X operator>>(const X& lhs, size_t n);
BOOST_AUTO_TEST_CASE(ShiftRight)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        composable::decrement_modulo()(a, k);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
