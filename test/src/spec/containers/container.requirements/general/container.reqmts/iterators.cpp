//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // mem_begin_end, mem_cbegin_cend, op_iterator_compare
#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/container.hpp>       // all, objects
#include <test/spec/input.hpp>           // context
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <utility>                       // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Iterators)

using namespace test::container;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/27-28,30-31: b.begin(), b.end()
BOOST_AUTO_TEST_CASE(BeginEnd)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        auto x                = a;
                        mem_begin_end()(x);
                        mem_begin_end()(std::as_const(x));
                }
        });
}

// [container.reqmts]/33-34,36-37: b.cbegin(), b.cend()
BOOST_AUTO_TEST_CASE(CbeginCend)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_cbegin_cend()(a);
                }
        });
}

// [container.reqmts]/39-40,63: i <=> j, and an iterator against a constant one
BOOST_AUTO_TEST_CASE(IteratorComparison)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        op_iterator_compare()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
