//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // fn_empty, fn_iterator, fn_size, fn_ssize
#include <test/spec/input.hpp>      // context, on_copy
#include <test/spec/set.hpp>        // all, sets
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Iterators)
BOOST_AUTO_TEST_SUITE(IteratorRange)

using namespace test::set;
using test::spec::context;
using test::spec::on_copy;
namespace inputs = test::spec::set::inputs;

// [iterator.range]/2-15: begin(c), end(c), cbegin(c), cend(c), rbegin(c), rend(c), crbegin(c), crend(c)
BOOST_AUTO_TEST_CASE(BeginEnd)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        on_copy(fn_iterator(), a);
                        fn_iterator()(a);
                }
        });
}

// [iterator.range]/16: size(const C& c)
BOOST_AUTO_TEST_CASE(Size)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        fn_size()(a);
                }
        });
}

// [iterator.range]/18: ssize(const C& c)
BOOST_AUTO_TEST_CASE(Ssize)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        fn_ssize()(a);
                }
        });
}

// [iterator.range]/20: empty(const C& c)
BOOST_AUTO_TEST_CASE(Empty)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        fn_empty()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
