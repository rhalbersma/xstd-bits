//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // constructor_copy, constructor_default, constructor_move, op_copy_assign, op_move_assign
#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/container.hpp>       // all, objects, pairs
#include <test/spec/input.hpp>           // context
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <utility>                       // move

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Construction)

using namespace test::container;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/10: X u; X u = X();
BOOST_AUTO_TEST_CASE(DefaultConstructor)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires { T(); });
                constructor_default<T>()();
        });
}

// [container.reqmts]/13: X u(v); X u = v;
BOOST_AUTO_TEST_CASE(CopyConstructor)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T const cc) { T(cc); });
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        constructor_copy()(a);
                }
        });
}

// [container.reqmts]/15: X u(rv); X u = rv;
BOOST_AUTO_TEST_CASE(MoveConstructor)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T o) { T(std::move(o)); });
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        constructor_move()(a);
                }
        });
}

// [container.reqmts]/17-18: t = v
BOOST_AUTO_TEST_CASE(CopyAssignment)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) { c = cc; });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_copy_assign()(a, b);
                }
        });
}

// [container.reqmts]/20-22: t = rv
BOOST_AUTO_TEST_CASE(MoveAssignment)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c, T o) { c = std::move(o); });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        op_move_assign()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
