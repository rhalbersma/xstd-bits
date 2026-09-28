//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // compares_as_a_constant, op_three_way
#include <test/for_each_type.hpp>        // for_each_type
#include <test/set/primitives.hpp>       // op_compare_three_way, op_greater, op_greater_equal, op_less, op_less_equal
#include <test/spec/container.hpp>       // all, constant_evaluable_v, keyed, objects, pairs, pairs_with_doubletons
#include <test/spec/input.hpp>           // context
#include <test/spec/set.hpp>             // triples
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                      // totally_ordered

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerOptReqmts)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.opt.reqmts]/1-2,4: a <=> b
BOOST_AUTO_TEST_CASE(ThreeWayComparison)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(std::totally_ordered<T>);
                if constexpr (test::spec::container::constant_evaluable_v<T>) {
                        static_assert(test::container::compares_as_a_constant<T>()); // [container.opt.reqmts]/1
                }
                // The operators it rewrites into follow from it, so doubleton pairs check the three-way result alone.
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        op_compare_three_way()(a);
                        op_less()(a);
                }
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_compare_three_way()(a, b);
                        test::container::op_three_way()(a, b);
                }
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_less()(a, b);
                        op_greater()(a, b);
                        op_less_equal()(a, b);
                        op_greater_equal()(a, b);
                }
                if constexpr (test::spec::container::keyed<T>) {
                        for (auto const [from, a, b, c] : test::spec::set::inputs::triples<T>()) {
                                auto const on_failure = context(from, a, b, c);
                                op_less()(a, b, c);
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
