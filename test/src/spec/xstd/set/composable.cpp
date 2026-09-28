//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/composable.hpp>  // includes, set_difference, set_intersection, set_symmetric_difference, set_union
#include <test/spec/input.hpp>      // context
#include <test/spec/set.hpp>        // all, pairs, pairs_with_doubletons
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Composable)

using namespace test::set;
using test::spec::context;
namespace inputs = test::spec::set::inputs;

// Each member operator is its algorithm over the keys, which a set without the member skips.

// xstd set: constexpr bool is_subset_of(const X& other) const;
BOOST_AUTO_TEST_CASE(IsSubsetOf)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::includes()(a, b);
                }
        });
}

// xstd set: constexpr X operator|(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitOr)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_union()(a, b);
                }
        });
}

// xstd set: constexpr X operator&(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitAnd)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_intersection()(a, b);
                }
        });
}

// xstd set: constexpr X operator-(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(Minus)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_difference()(a, b);
                }
        });
}

// xstd set: constexpr X operator^(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitXor)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_symmetric_difference()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
