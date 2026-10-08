//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/composable.hpp>  // proper_subset, proper_superset, set_difference, set_intersection, set_symmetric_difference, set_union, subset, superset
#include <test/set/exhaustive.hpp>  // static_width
#include <test/spec/input.hpp>      // context
#include <test/spec/rejection.hpp>  // has_and_assign, has_bit_and, has_bit_or, has_bit_xor, has_complement, has_minus, has_minus_assign, has_or_assign, has_xor_assign
#include <test/spec/set.hpp>        // all, const_views, owners, pairs, pairs_with_doubletons, views
#include <test/spec/view.hpp>       // owner_t
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK

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
                        composable::subset()(a, b);
                }
        });
}

// xstd set: constexpr bool is_proper_subset_of(const X& other) const;
BOOST_AUTO_TEST_CASE(IsProperSubsetOf)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::proper_subset()(a, b);
                }
        });
}

// xstd set: constexpr bool is_superset_of(const X& other) const;
BOOST_AUTO_TEST_CASE(IsSupersetOf)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::superset()(a, b);
                }
        });
}

// xstd set: constexpr bool is_proper_superset_of(const X& other) const;
BOOST_AUTO_TEST_CASE(IsProperSupersetOf)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs_with_doubletons<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::proper_superset()(a, b);
                }
        });
}

// xstd set: constexpr X operator|(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitOr)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_union()(a, b);
                }
        });
        // A view writes in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_or_assign<T> and test::spec::has_bit_or<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_bit_or<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_or_assign<T>); });
}

// xstd set: constexpr X operator&(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitAnd)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_intersection()(a, b);
                }
        });
        // A view writes in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_and_assign<T> and test::spec::has_bit_and<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_bit_and<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_and_assign<T>); });
}

// xstd set: constexpr X operator-(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(Minus)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_difference()(a, b);
                }
        });
        // A view writes in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_minus_assign<T> and test::spec::has_minus<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_minus<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_minus_assign<T>); });
}

// xstd set: constexpr X operator^(const X& lhs, const X& rhs);
BOOST_AUTO_TEST_CASE(BitXor)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        composable::set_symmetric_difference()(a, b);
                }
        });
        // A view writes in place, and copies no keys into a value it could return, by this library's design.
        test::for_each_type<test::spec::set::views>([]<class T> -> void {
                static_assert(test::spec::has_xor_assign<T> and test::spec::has_bit_xor<test::spec::owner_t<T>>);
                static_assert(not test::spec::has_bit_xor<T>);
        });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_xor_assign<T>); });
}

// xstd set: constexpr X operator~(const X& lhs);
BOOST_AUTO_TEST_CASE(Complement)
{
        // Only an owner's width in the type is a universe to complement in, by this library's design.
        test::for_each_type<test::spec::set::all>([]<class T> -> void { static_assert(test::spec::has_complement<T> == test::set::static_width<T>); });
        test::for_each_type<test::spec::set::const_views>([]<class T> -> void { static_assert(not test::spec::has_complement<T>); });
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
