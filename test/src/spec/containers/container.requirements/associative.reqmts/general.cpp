//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>        // is_flat_set
#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // mem_clear, mem_contains, mem_count, mem_emplace, mem_emplace_hint, mem_equal_range, mem_erase, mem_find, mem_insert, mem_lower_bound, mem_upper_bound
#include <test/spec/input.hpp>      // context, with_initializer_list
#include <test/spec/set.hpp>        // all, keyed_sets, keyed_sets_with_singletons, listed_sets, sets, sets_with_doubletons
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as
#include <cstddef>                  // size_t
#include <initializer_list>         // initializer_list
#include <utility>                  // as_const, pair
#include <version>                  // IWYU pragma: keep; __cpp_lib_containers_ranges

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(AssociativeReqmts)
BOOST_AUTO_TEST_SUITE(General)

using namespace test::set;
using test::spec::context;
using test::spec::with_initializer_list;
namespace inputs = test::spec::set::inputs;

// [associative.reqmts.general]/9-16: typename X::key_type, X::value_type, X::key_compare, X::value_compare
BOOST_AUTO_TEST_CASE(NestedTypes)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires {
                        typename T::key_type;
                        typename T::value_type;
                        typename T::key_compare;
                        typename T::value_compare;
                });
                BOOST_CHECK(true);
        });
}

// [associative.reqmts.general]/41-43: b.key_comp()
BOOST_AUTO_TEST_CASE(KeyComp)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.key_comp() } -> std::same_as<typename T::key_compare>; });
                auto const comp = T(typename T::key_compare()).key_comp();
                BOOST_CHECK(comp(0UZ, 1UZ) and not comp(1UZ, 0UZ) and not comp(1UZ, 1UZ));
        });
}

// [associative.reqmts.general]/44-46: b.value_comp()
BOOST_AUTO_TEST_CASE(ValueComp)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.value_comp() } -> std::same_as<typename T::value_compare>; });
                auto const comp = T().value_comp();
                BOOST_CHECK(comp(0UZ, 1UZ) and not comp(1UZ, 0UZ) and not comp(1UZ, 1UZ));
        });
}

// [associative.reqmts.general]/47-51: a_uniq.emplace(args)
BOOST_AUTO_TEST_CASE(Emplace)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k) {
                        { c.emplace(k) } -> std::same_as<std::pair<typename T::iterator, bool>>;
                        { c.emplace() } -> std::same_as<std::pair<typename T::iterator, bool>>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_emplace()(x, k);
                }
        });
}

// [associative.reqmts.general]/57-60: a.emplace_hint(p, args)
BOOST_AUTO_TEST_CASE(EmplaceHint)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k, T::const_iterator p) {
                        { c.emplace_hint(p, k) } -> std::same_as<typename T::iterator>;
                        { c.emplace_hint(p) } -> std::same_as<typename T::iterator>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_emplace_hint()(x, x.end(), k);
                }
        });
}

// [associative.reqmts.general]/61-65: a_uniq.insert(t)
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k) { { c.insert(k) } -> std::same_as<std::pair<typename T::iterator, bool>>; });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_insert()(x, k);
                }
        });
}

// [associative.reqmts.general]/70-74: a.insert(p, t)
BOOST_AUTO_TEST_CASE(InsertHint)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k, T::const_iterator p) { { c.insert(p, k) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_insert()(x, x.end(), k);
                }
        });
}

// [associative.reqmts.general]/75-78: a.insert(i, j)
BOOST_AUTO_TEST_CASE(InsertFirstLast)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::value_type const* first, T::value_type const* last) { c.insert(first, last); });
                for (auto const [from, a, keys] : inputs::listed_sets<T>()) {
                        auto const on_failure = context(from, a, keys);
                        auto x = a;
                        mem_insert()(x, keys.begin(), keys.end());
                }
        });
}

// [associative.reqmts.general]/79-82: a.insert_range(rg)
BOOST_AUTO_TEST_CASE(InsertRange)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (T c, std::initializer_list<typename T::value_type> il) { c.insert_range(il); });
                for (auto const [from, a, keys] : inputs::listed_sets<T>()) {
                        auto const on_failure = context(from, a, keys);
                        auto x = a;
                        mem_insert()(x, keys);
                }
#endif
                BOOST_CHECK(true);
        });
}

// [associative.reqmts.general]/83: a.insert(il)
BOOST_AUTO_TEST_CASE(InsertInitializerList)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, std::initializer_list<typename T::value_type> il) { c.insert(il); });
                for (auto const [from, a, keys] : inputs::listed_sets<T>()) {
                        auto const on_failure = context(from, a, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                auto x = a;
                                mem_insert()(x, il);
                        });
                }
        });
}

// [associative.reqmts.general]/118-121: a.erase(k)
BOOST_AUTO_TEST_CASE(EraseKey)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k) { { c.erase(k) } -> std::same_as<typename T::size_type>; });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_erase()(x, k);
                }
        });
}

// [associative.reqmts.general]/126-129: a.erase(q)
BOOST_AUTO_TEST_CASE(EraseIterator)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::const_iterator p) { { c.erase(p) } -> std::same_as<typename T::iterator>; });
                // std::flat_set's erase invalidates the iterators past it, so it is not erased from one by one.
                if constexpr (not test::is_flat_set<T>) {
                        for (auto const [from, a] : inputs::sets<T>()) {
                                auto const on_failure = context(from, a);
                                auto x = a;
                                for (auto first = x.begin(), last = x.end(); first != last; /* expression inside loop */) {
                                        mem_erase()(x, first++);
                                }
                        }
                }
        });
}

// [associative.reqmts.general]/134-137: a.erase(q1, q2)
BOOST_AUTO_TEST_CASE(EraseRange)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::const_iterator p) { { c.erase(p, p) } -> std::same_as<typename T::iterator>; });
                if constexpr (not test::is_flat_set<T>) {
                        for (auto const [from, a] : inputs::sets_with_doubletons<T>()) {
                                auto const on_failure = context(from, a);
                                auto x = a;
                                mem_erase()(x, x.begin(), x.end());
                        }
                }
        });
}

// [associative.reqmts.general]/138-140: a.clear()
BOOST_AUTO_TEST_CASE(Clear)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c) { c.clear(); });
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        auto x = a;
                        mem_clear()(x);
                }
        });
}

// [associative.reqmts.general]/141-143: b.find(k)
BOOST_AUTO_TEST_CASE(Find)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc, T::key_type k) {
                        { c.find(k) } -> std::same_as<typename T::iterator>;
                        { cc.find(k) } -> std::same_as<typename T::const_iterator>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_find()(x, k);
                        mem_find()(std::as_const(x), k);
                }
        });
}

// [associative.reqmts.general]/147-149: b.count(k)
BOOST_AUTO_TEST_CASE(Count)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T const cc, T::key_type k) { { cc.count(k) } -> std::same_as<typename T::size_type>; });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto const x = a;
                        mem_count()(x, k);
                }
        });
}

// [associative.reqmts.general]/153-154: b.contains(k)
BOOST_AUTO_TEST_CASE(Contains)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T const cc, T::key_type k) { { cc.contains(k) } -> std::same_as<bool>; });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto const x = a;
                        mem_contains()(x, k);
                }
        });
}

// [associative.reqmts.general]/157-159: b.lower_bound(k)
BOOST_AUTO_TEST_CASE(LowerBound)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc, T::key_type k) {
                        { c.lower_bound(k) } -> std::same_as<typename T::iterator>;
                        { cc.lower_bound(k) } -> std::same_as<typename T::const_iterator>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_lower_bound()(x, k);
                        mem_lower_bound()(std::as_const(x), k);
                }
        });
}

// [associative.reqmts.general]/163-165: b.upper_bound(k)
BOOST_AUTO_TEST_CASE(UpperBound)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc, T::key_type k) {
                        { c.upper_bound(k) } -> std::same_as<typename T::iterator>;
                        { cc.upper_bound(k) } -> std::same_as<typename T::const_iterator>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_upper_bound()(x, k);
                        mem_upper_bound()(std::as_const(x), k);
                }
        });
}

// [associative.reqmts.general]/169-171: b.equal_range(k)
BOOST_AUTO_TEST_CASE(EqualRange)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc, T::key_type k) {
                        { c.equal_range(k) } -> std::same_as<std::pair<typename T::iterator, typename T::iterator>>;
                        { cc.equal_range(k) } -> std::same_as<std::pair<typename T::const_iterator, typename T::const_iterator>>;
                });
                for (auto const [from, a, k] : inputs::keyed_sets_with_singletons<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_equal_range()(x, k);
                        mem_equal_range()(std::as_const(x), k);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
