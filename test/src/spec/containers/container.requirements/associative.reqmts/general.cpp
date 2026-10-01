//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>        // is_flat_set
#include <test/for_each_type.hpp>   // for_each_type
#include <test/set/primitives.hpp>  // constructor, key_order, mem_clear, mem_contains, mem_count, mem_emplace, mem_emplace_hint, mem_equal_range, mem_erase, mem_erase_mutable, mem_find, mem_insert, mem_lower_bound, mem_upper_bound, nested_types, no_heterogeneous_members, op_assign, same_order
#include <test/spec/input.hpp>      // context, with_initializer_list
#include <test/spec/set.hpp>        // all, key_lists, keyed_sets, keyed_sets_with_singletons, listed_sets, owners, sets, sets_with_doubletons
#include <xstd/bits/bit_set.hpp>    // basic_bit_set, bit_set
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // same_as
#include <cstddef>                  // size_t
#include <functional>               // less
#include <initializer_list>         // initializer_list
#include <iterator>                 // prev
#include <memory>                   // allocator
#include <ranges>                   // from_range
#include <set>                      // set
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

// [associative.reqmts.general]/5-6,9,12-16: typename X::key_type, X::value_type, X::key_compare, X::value_compare
BOOST_AUTO_TEST_CASE(NestedTypes)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires {
                        typename T::key_type;
                        typename T::value_type;
                        typename T::key_compare;
                        typename T::value_compare;
                });
                nested_types<T>();
                BOOST_CHECK(true);
        });
}

// [associative.reqmts.general]/18: X(c)
BOOST_AUTO_TEST_CASE(Comp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                static_assert(requires (T::key_compare const c) { T(c); });
                constructor<T>()(typename T::key_compare());
        });
}

// [associative.reqmts.general]/20-21: X u = X(); X u;
BOOST_AUTO_TEST_CASE(Default)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                constructor<T>()();
                T const u;
                BOOST_CHECK(u.empty() and same_order(u.key_comp(), typename T::key_compare())); // [associative.reqmts.general]/21
        });
}

// [associative.reqmts.general]/23-24: X(i, j, c)
BOOST_AUTO_TEST_CASE(FirstLastComp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(keys.begin(), keys.end(), typename T::key_compare());
                }
        });
}

// [associative.reqmts.general]/26-27: X(i, j)
BOOST_AUTO_TEST_CASE(FirstLast)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(keys.begin(), keys.end());
                }
        });
}

// [associative.reqmts.general]/29-30: X(from_range, rg, c)
BOOST_AUTO_TEST_CASE(FromRangeComp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(std::from_range, keys, typename T::key_compare());
                }
        });
}

// [associative.reqmts.general]/32-33: X(from_range, rg)
BOOST_AUTO_TEST_CASE(FromRange)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        constructor<T>()(std::from_range, keys);
                }
        });
}

// [associative.reqmts.general]/35: X(il, c)
BOOST_AUTO_TEST_CASE(InitializerListComp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                constructor<T>()(il, typename T::key_compare());
                        });
                }
        });
}

// [associative.reqmts.general]/36: X(il)
BOOST_AUTO_TEST_CASE(InitializerList)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, keys] : inputs::key_lists<T>()) {
                        auto const on_failure = context(from, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                constructor<T>()(il);
                        });
                }
        });
}

// [associative.reqmts.general]/37-39: a = il
BOOST_AUTO_TEST_CASE(AssignInitializerList)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                for (auto const [from, a, keys] : inputs::listed_sets<T>()) {
                        auto const on_failure = context(from, a, keys);
                        with_initializer_list(keys, [&](std::initializer_list<std::size_t> il) -> void {
                                auto x = a;
                                op_assign()(x, il);
                        });
                }
        });
}

// [associative.reqmts.general]/41-42: b.key_comp()
BOOST_AUTO_TEST_CASE(KeyComp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.key_comp() } -> std::same_as<typename T::key_compare>; }); // [associative.reqmts.general]/41
                auto const c = typename T::key_compare();
                BOOST_CHECK(same_order(T(c).key_comp(), c)); // [associative.reqmts.general]/42
        });
}

// [associative.reqmts.general]/44-45: b.value_comp()
BOOST_AUTO_TEST_CASE(ValueComp)
{
        test::for_each_type<test::spec::set::owners>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.value_comp() } -> std::same_as<typename T::value_compare>; }); // [associative.reqmts.general]/44
                auto const c = typename T::key_compare();
                BOOST_CHECK(same_order(T(c).value_comp(), c)); // [associative.reqmts.general]/45
        });
}

// [associative.reqmts.general]/47-50: a_uniq.emplace(args)
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

// [associative.reqmts.general]/57-59: a.emplace_hint(p, args)
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

// [associative.reqmts.general]/4,61-64: a_uniq.insert(t)
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k) { { c.insert(k) } -> std::same_as<std::pair<typename T::iterator, bool>>; });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_insert()(x, k);
                        auto y = a;
                        mem_insert()(y, typename T::key_type{k});
                }
        });
}

// [associative.reqmts.general]/70-73: a.insert(p, t)
BOOST_AUTO_TEST_CASE(InsertHint)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::key_type k, T::const_iterator p) { { c.insert(p, k) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, k] : inputs::keyed_sets<T>()) {
                        auto const on_failure = context(from, a, k);
                        auto x = a;
                        mem_insert()(x, x.end(), k);
                        auto y = a;
                        mem_insert()(y, y.end(), typename T::key_type{k});
                }
        });
}

// [associative.reqmts.general]/75-77: a.insert(i, j)
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

// [associative.reqmts.general]/79-81: a.insert_range(rg)
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

// [associative.reqmts.general]/118-120: a.erase(k)
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

// [associative.reqmts.general]/126-128: a.erase(q)
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

// [associative.reqmts.general]/130-132: a.erase(r)
BOOST_AUTO_TEST_CASE(EraseMutableIterator)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                static_assert(requires (T c, T::iterator r) { { c.erase(r) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a] : inputs::sets<T>()) {
                        auto const on_failure = context(from, a);
                        if (not a.empty()) {
                                auto x = a;
                                mem_erase_mutable()(x, x.begin());
                                auto y = a;
                                mem_erase_mutable()(y, std::prev(y.end()));
                        }
                }
        });
}

// [associative.reqmts.general]/134-136: a.erase(q1, q2)
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

// [associative.reqmts.general]/138-139: a.clear()
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

// [associative.reqmts.general]/141-142: b.find(k)
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

// [associative.reqmts.general]/147-148: b.count(k)
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

// [associative.reqmts.general]/157-158: b.lower_bound(k)
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

// [associative.reqmts.general]/163-164: b.upper_bound(k)
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

// [associative.reqmts.general]/169-170: b.equal_range(k)
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

// [associative.reqmts.general]/177-178: iterators over the keys in order
BOOST_AUTO_TEST_CASE(KeyOrder)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sets_with_doubletons<T>()) {
                        auto const on_failure = context(from, a);
                        key_order()(a);
                }
        });
}

// [associative.reqmts.general]/180: the member function templates for heterogeneous keys
BOOST_AUTO_TEST_CASE(HeterogeneousMembers)
{
        test::for_each_type<test::spec::set::all>([]<class T> -> void {
                no_heterogeneous_members<T>();
                BOOST_CHECK(true);
        });
}

namespace {

// What each of the two guided class templates deduces, or cannot, from an iterator pair and one more argument.
template<class I, class... Args>
concept deduces_std_set = requires (I i, Args... args) { std::set(i, i, args...); };

template<class I, class... Args>
concept deduces_bit_set = requires (I i, Args... args) { xstd::basic_bit_set(i, i, args...); };

} // namespace

// [associative.reqmts.general]/181: deduction guides
BOOST_AUTO_TEST_CASE(DeductionGuides)
{
        using I = std::size_t const*;
        using Alloc = std::allocator<std::size_t>;
        using Less = std::less<std::size_t>;

        static_assert(deduces_std_set<I> and not deduces_std_set<int>);                            // [associative.reqmts.general]/181
        static_assert(deduces_std_set<I, Less, Alloc> and not deduces_std_set<I, Less, int>);      // [associative.reqmts.general]/181
        static_assert(std::same_as<decltype(std::set(I(), I(), Alloc())), std::set<std::size_t>>); // [associative.reqmts.general]/181

        static_assert(deduces_bit_set<I> and not deduces_bit_set<int>);                               // [associative.reqmts.general]/181
        static_assert(deduces_bit_set<I, Less, Alloc> and not deduces_bit_set<I, Less, int>);         // [associative.reqmts.general]/181
        static_assert(std::same_as<decltype(xstd::basic_bit_set(I(), I(), Alloc())), xstd::bit_set>); // [associative.reqmts.general]/181
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
