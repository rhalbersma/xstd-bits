//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // mem_empty, mem_max_size, mem_size
#include <test/for_each_type.hpp>        // for_each_type
#include <test/spec/container.hpp>       // all, objects
#include <test/spec/input.hpp>           // context
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                      // same_as

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Size)

using namespace test::container;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/52-53: c.size()
BOOST_AUTO_TEST_CASE(Size)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.size() } -> std::same_as<typename T::size_type>; });
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_size()(a);
                }
        });
}

// [container.reqmts]/56-57: c.max_size()
BOOST_AUTO_TEST_CASE(MaxSize)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.max_size() } -> std::same_as<typename T::size_type>; });
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_max_size()(a);
                }
        });
}

// [container.reqmts]/59-60,62: c.empty()
BOOST_AUTO_TEST_CASE(Empty)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.empty() } -> std::same_as<bool>; });
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_empty()(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
