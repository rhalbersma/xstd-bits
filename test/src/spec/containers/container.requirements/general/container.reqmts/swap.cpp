//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // fn_swap, mem_swap
#include <test/spec/container.hpp>  // all, pairs
#include <test/spec/input.hpp>      // context
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Swap)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/48-50: t.swap(s)
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto x = a;
                        auto y = b;
                        mem_swap()(x, y);
                }
        });
}

// [container.reqmts]/51: swap(t, s)
BOOST_AUTO_TEST_CASE(NonMemberSwap)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto x = a;
                        auto y = b;
                        fn_swap()(x, y);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
