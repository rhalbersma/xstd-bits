//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/spec/container.hpp>  // all
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as
#include <iterator>                 // reverse_iterator

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerRevReqmts)

// [container.rev.reqmts]/2: typename X::reverse_iterator
BOOST_AUTO_TEST_CASE(ReverseIterator)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(std::same_as<typename T::reverse_iterator, std::reverse_iterator<typename T::iterator>>);
                BOOST_CHECK(true);
        });
}

// [container.rev.reqmts]/3: typename X::const_reverse_iterator
BOOST_AUTO_TEST_CASE(ConstReverseIterator)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(std::same_as<typename T::const_reverse_iterator, std::reverse_iterator<typename T::const_iterator>>);
                BOOST_CHECK(true);
        });
}

// [container.rev.reqmts]/4-6: a.rbegin()
BOOST_AUTO_TEST_CASE(Rbegin)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) {
                        { c.rbegin() } -> std::same_as<typename T::reverse_iterator>;
                        { cc.rbegin() } -> std::same_as<typename T::const_reverse_iterator>;
                });
                auto const c = T();
                BOOST_CHECK(c.rbegin() == typename T::const_reverse_iterator(c.end())); // [container.rev.reqmts]/5
        });
}

// [container.rev.reqmts]/7-9: a.rend()
BOOST_AUTO_TEST_CASE(Rend)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) {
                        { c.rend() } -> std::same_as<typename T::reverse_iterator>;
                        { cc.rend() } -> std::same_as<typename T::const_reverse_iterator>;
                });
                auto const c = T();
                BOOST_CHECK(c.rend() == typename T::const_reverse_iterator(c.begin())); // [container.rev.reqmts]/8
        });
}

// [container.rev.reqmts]/10-12: a.crbegin()
BOOST_AUTO_TEST_CASE(Crbegin)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c) { { c.crbegin() } -> std::same_as<typename T::const_reverse_iterator>; });
                auto const c = T();
                BOOST_CHECK(c.crbegin() == c.rbegin()); // [container.rev.reqmts]/11
        });
}

// [container.rev.reqmts]/13-15: a.crend()
BOOST_AUTO_TEST_CASE(Crend)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c) { { c.crend() } -> std::same_as<typename T::const_reverse_iterator>; });
                auto const c = T();
                BOOST_CHECK(c.crend() == c.rend()); // [container.rev.reqmts]/14
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
