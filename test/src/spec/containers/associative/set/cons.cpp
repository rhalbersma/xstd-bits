//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // constructor, op_assign
#include <test/spec/input.hpp>      // context, on_copy, with_initializer_list
#include <test/spec/set.hpp>        // all, key_lists, listed_sets_with_singletons, pairs, sets
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <cstddef>                  // size_t
#include <initializer_list>         // initializer_list
#include <ranges>                   // from_range
#include <utility>                  // move

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Associative)
BOOST_AUTO_TEST_SUITE(Set)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::set;
using test::spec::context;
using test::spec::on_copy;
using test::spec::with_initializer_list;
namespace inputs = test::spec::set::inputs;

// [set.overview]: constexpr set() : set(Compare()) { }
BOOST_AUTO_TEST_CASE(Set)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires { T(); });
                constructor<T>()();
        });
}

// [set.cons]/1-2: constexpr explicit set(const Compare& comp, const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(SetComp)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T::key_compare const comp) { T(comp); });
                // std::less has no state, so a comparator argument is accepted and changes nothing.
                BOOST_CHECK(T(typename T::key_compare()).empty());
        });
}

// [set.cons]/3-4: set(InputIterator first, InputIterator last, const Compare& comp = Compare(), ...);
BOOST_AUTO_TEST_CASE(SetFirstLast)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T::key_compare const comp, T::value_type const* first, T::value_type const* last) {
                        T(first, last);
                        T(first, last, comp);
                });
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(keys.begin(), keys.end());
                        BOOST_CHECK(T(keys.begin(), keys.end(), typename T::key_compare()) == T(keys.begin(), keys.end()));
                }
        });
}

// [set.cons]/5-6: set(from_range_t, R&& rg, const Compare& comp = Compare(), const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(SetFromRange)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T::key_compare const comp, std::initializer_list<typename T::value_type> il) {
                        T(std::from_range, il);
                        T(std::from_range, il, comp);
                });
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(std::from_range, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                constructor<T>()(std::from_range, il);
                        });
                }
        });
}

// [set.overview]: constexpr set(const set& x);
BOOST_AUTO_TEST_CASE(SetCopy)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T const cc) { T(cc); });
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        constructor<T>()(a);
                }
        });
}

// [set.overview]: constexpr set(set&& x);
BOOST_AUTO_TEST_CASE(SetMove)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T o) { T(std::move(o)); });
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        auto rv = a;
                        auto const u = T(std::move(rv));
                        BOOST_CHECK(u == a);
                }
        });
}

// [set.overview]: set(initializer_list<value_type>, const Compare& = Compare(), const Allocator& = Allocator());
BOOST_AUTO_TEST_CASE(SetInitializerList)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T::key_compare const comp, std::initializer_list<typename T::value_type> il) {
                        T(il);
                        T(il, comp);
                });
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                constructor<T>()(il);
                                BOOST_CHECK(T(il, typename T::key_compare()) == T(il));
                        });
                }
        });
}

// [set.overview]: constexpr set& operator=(const set& x);
BOOST_AUTO_TEST_CASE(Assign)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) { c = cc; });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        on_copy(op_assign(), a, b);
                }
        });
}

// [set.overview]: constexpr set& operator=(set&& x) noexcept(...);
BOOST_AUTO_TEST_CASE(AssignMove)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T o) { c = std::move(o); });
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto t = a;
                        auto rv = b;
                        t = std::move(rv);
                        BOOST_CHECK(t == b);
                }
        });
}

// [set.overview]: constexpr set& operator=(initializer_list<value_type>);
BOOST_AUTO_TEST_CASE(AssignInitializerList)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, std::initializer_list<typename T::value_type> il) { c = il; });
                for (auto const [from, a, keys] : inputs::listed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                on_copy(op_assign(), a, il);
                        });
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
