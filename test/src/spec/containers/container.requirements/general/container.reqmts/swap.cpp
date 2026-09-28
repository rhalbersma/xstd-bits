//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // fn_swap, mem_swap
#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/container.hpp>       // all, pairs
#include <test/spec/input.hpp>           // context
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Swap)

using namespace test::container;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/48-49: t.swap(s)
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c) { c.swap(c); });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_swap()(a, b);
                }
        });
}

// [container.reqmts]/51: swap(t, s)
BOOST_AUTO_TEST_CASE(NonMemberSwap)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c) { swap(c, c); });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        fn_swap()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
