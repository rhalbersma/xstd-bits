//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // mem_crbegin, mem_crend, mem_rbegin, mem_rend
#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/container.hpp>       // all, objects
#include <test/spec/input.hpp>           // context
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                      // same_as
#include <iterator>                      // bidirectional_iterator, iter_value_t, reverse_iterator

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerRevReqmts)

using namespace test::container;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.rev.reqmts]/1-2: typename X::reverse_iterator
BOOST_AUTO_TEST_CASE(ReverseIterator)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(std::bidirectional_iterator<typename T::iterator>);                                       // [container.rev.reqmts]/1
                static_assert(std::same_as<typename T::reverse_iterator, std::reverse_iterator<typename T::iterator>>); // [container.rev.reqmts]/2
                static_assert(std::same_as<std::iter_value_t<typename T::reverse_iterator>, typename T::value_type>);   // [container.rev.reqmts]/2
                BOOST_CHECK(true);
        });
}

// [container.rev.reqmts]/3: typename X::const_reverse_iterator
BOOST_AUTO_TEST_CASE(ConstReverseIterator)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(std::same_as<typename T::const_reverse_iterator, std::reverse_iterator<typename T::const_iterator>>); // [container.rev.reqmts]/3
                static_assert(std::same_as<std::iter_value_t<typename T::const_reverse_iterator>, typename T::value_type>);         // [container.rev.reqmts]/3
                BOOST_CHECK(true);
        });
}

// [container.rev.reqmts]/4-5: a.rbegin()
BOOST_AUTO_TEST_CASE(Rbegin)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_rbegin()(a);
                }
        });
}

// [container.rev.reqmts]/7-8: a.rend()
BOOST_AUTO_TEST_CASE(Rend)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_rend()(a);
                }
        });
}

// [container.rev.reqmts]/10-11: a.crbegin()
BOOST_AUTO_TEST_CASE(Crbegin)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_crbegin()(a);
                }
        });
}

// [container.rev.reqmts]/13-14: a.crend()
BOOST_AUTO_TEST_CASE(Crend)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_crend()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
