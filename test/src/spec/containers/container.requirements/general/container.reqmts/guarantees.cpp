//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/container/primitives.hpp> // mem_erasers, mem_observers
#include <test/for_each_type.hpp>        // for_each_type
#include <test/sequence/factory.hpp>     // model_of
#include <test/spec/container.hpp>       // all, objects
#include <test/spec/input.hpp>           // context
#include <test/spec/sequence.hpp>        // growable_all
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <vector>                        // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Guarantees)

using namespace test::container;
using test::sequence::model_of;
using test::spec::context;
namespace inputs = test::spec::container::inputs;

// [container.reqmts]/66: no swap(), clear(), erase() or pop_back() throws
BOOST_AUTO_TEST_CASE(NoThrow)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                static_assert(requires (T c) { { c.swap(c) } noexcept; }); // [container.reqmts]/66
                if constexpr (requires (T c) { c.clear(); }) {
                        for (auto const [from, a] : inputs::objects<T>()) {
                                auto const on_failure = context(from, a);
                                mem_erasers()(a);
                        }
                }
        });
}

// [container.reqmts]/67: a member called for what it reports
BOOST_AUTO_TEST_CASE(Observers)
{
        test::for_each_type<test::spec::container::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::objects<T>()) {
                        auto const on_failure = context(from, a);
                        mem_observers()(a);
                }
        });
}

// [container.reqmts]/69: two integers are a count and a value, never a pair of iterators
BOOST_AUTO_TEST_CASE(IntegralArguments)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                if (T().max_size() >= 3UZ) {
                        BOOST_CHECK(model_of(T(3, 1)) == std::vector<bool>(3, true));    // [container.reqmts]/69
                        BOOST_CHECK(model_of(T(2U, 0L)) == std::vector<bool>(2, false)); // [container.reqmts]/69
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
