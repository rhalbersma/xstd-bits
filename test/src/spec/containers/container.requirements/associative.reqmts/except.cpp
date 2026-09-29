//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // mem_clear_erase_nothrow, mem_insert_or_nothing, mem_swap_nothrow
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, keyed_sets, pairs, sets
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(AssociativeReqmts)
BOOST_AUTO_TEST_SUITE(Except)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// [associative.reqmts.except]/1: a.clear(), a.erase(k)
BOOST_AUTO_TEST_CASE(ClearErase)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        mem_clear_erase_nothrow()(a, k);
                }
        });
}

// [associative.reqmts.except]/2: a.insert(t), a.insert(p, t), a.emplace(args), a.emplace_hint(p, args)
BOOST_AUTO_TEST_CASE(InsertEmplace)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                // A key past what the set can hold is the one insertion an xstd set refuses by throwing.
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        mem_insert_or_nothing()(a, a.max_size());
                }
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        mem_insert_or_nothing()(a, k);
                }
        });
}

// [associative.reqmts.except]/3: a.swap(b), swap(a, b)
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        mem_swap_nothrow()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
