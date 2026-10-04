//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/factory.hpp>    // model_of
#include <test/sequence/primitives.hpp> // alternating, constructor, mem_append_range, mem_assign, mem_assign_range, mem_at, mem_back, mem_clear, mem_emplace, mem_emplace_back, mem_erase, mem_front, mem_insert, mem_insert_range, mem_pop_back, mem_push_back, mem_subscript, op_assign, reads_once, unsized_alternating
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // all, growable_all, indexed, positions, prefixes, sequences, spans
#include <xstd/bits/bit_vector.hpp>     // basic_bit_vector, bit_vector
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                     // same_as
#include <cstddef>                      // size_t
#include <initializer_list>             // initializer_list
#include <memory>                       // allocator
#include <ranges>                       // from_range
#include <type_traits>                  // type_identity
#include <vector>                       // vector
#include <version>                      // IWYU pragma: keep; __cpp_lib_containers_ranges

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(SequenceReqmts)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [sequence.reqmts]/6-7: X u(n, t);
BOOST_AUTO_TEST_CASE(CountConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T::size_type n, bool b) { T(n, b); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        constructor<T>()(a.size(), false);
                        constructor<T>()(a.size(), true);
                }
        });
}

// [sequence.reqmts]/9-10: X u(i, j);
BOOST_AUTO_TEST_CASE(IteratorConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (bool const* first, bool const* last) { T(first, last); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto const sized      = model_of(a);
                        auto unsized          = unsized_alternating(a.size());
                        constructor<T>()(sized.begin(), sized.end());
                        constructor<T>()(unsized.begin(), unsized.end());
                        reads_once()(std::type_identity<T>(), sized);
                }
        });
}

// [sequence.reqmts]/12,14: X(from_range, rg)
BOOST_AUTO_TEST_CASE(RangeConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (std::initializer_list<bool> il) { T(std::from_range, il); });
#endif
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        constructor<T>()(std::from_range, model_of(a));
                        constructor<T>()(std::from_range, unsized_alternating(a.size()));
                }
        });
}

// [sequence.reqmts]/15: X(il)
BOOST_AUTO_TEST_CASE(InitializerListConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (std::initializer_list<bool> il) { T(il); });
                constructor<T>()({});
                constructor<T>()({true});
                constructor<T>()({true, false, true});
                constructor<T>()({true, false, true, false, true, false, true, false});
                constructor<T>()({true, false, true, false, true, false, true, false, true});
        });
}

// [sequence.reqmts]/16,18-19: a = il
BOOST_AUTO_TEST_CASE(InitializerListAssignment)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, std::initializer_list<bool> il) { c = il; });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        op_assign()(a, {});
                        op_assign()(a, {false, true});
                }
        });
}

// [sequence.reqmts]/20,22-23: a.emplace(p, args)
BOOST_AUTO_TEST_CASE(Emplace)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool b, T::const_iterator p) {
                        { c.emplace(p, b) } -> std::same_as<typename T::iterator>;
                        { c.emplace(p) } -> std::same_as<typename T::iterator>;
                });
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_emplace()(a, p, false);
                        mem_emplace()(a, p, true);
                        mem_emplace()(a, p);
                }
        });
}

// [sequence.reqmts]/24,26-28,30-31: a.insert(p, t), a.insert(p, rv)
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool b, T::const_iterator p) { { c.insert(p, b) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert()(a, p, false);
                        mem_insert()(a, p, true);
                }
        });
}

// [sequence.reqmts]/32,34-35: a.insert(p, n, t)
BOOST_AUTO_TEST_CASE(InsertCount)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n, bool b, T::const_iterator p) { { c.insert(p, n, b) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        mem_insert()(a, p, n, true);
                }
        });
}

// [sequence.reqmts]/36,38-39: a.insert(p, i, j)
BOOST_AUTO_TEST_CASE(InsertIterators)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool const* first, bool const* last, T::const_iterator p) { { c.insert(p, first, last) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        auto const sized      = alternating(n);
                        auto unsized          = unsized_alternating(n);
                        mem_insert()(a, p, sized.begin(), sized.end());
                        mem_insert()(a, p, unsized.begin(), unsized.end());
                        reads_once()(a, p, sized);
                }
        });
}

// [sequence.reqmts]/40,42-43: a.insert_range(p, rg)
BOOST_AUTO_TEST_CASE(InsertRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (T c, std::initializer_list<bool> il, T::const_iterator p) { { c.insert_range(p, il) } -> std::same_as<typename T::iterator>; });
#endif
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        mem_insert_range()(a, p, alternating(n));
                        mem_insert_range()(a, p, unsized_alternating(n));
                }
        });
}

// [sequence.reqmts]/44: a.insert(p, il)
BOOST_AUTO_TEST_CASE(InsertInitializerList)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, std::initializer_list<bool> il, T::const_iterator p) { { c.insert(p, il) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert()(a, p, {true, false, true});
                        mem_insert()(a, p, {true, false, true, false, true, false, true, false});
                        mem_insert()(a, p, {true, false, true, false, true, false, true, false, true});
                }
        });
}

// [sequence.reqmts]/45,47-48: a.erase(q)
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, T::const_iterator p) { { c.erase(p) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_erase()(a, p);
                }
        });
}

// [sequence.reqmts]/49,51-52: a.erase(q1, q2)
BOOST_AUTO_TEST_CASE(EraseRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, T::const_iterator p) { { c.erase(p, p) } -> std::same_as<typename T::iterator>; });
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        mem_erase()(a, p, p + n);
                }
        });
}

// [sequence.reqmts]/53-55: a.clear()
BOOST_AUTO_TEST_CASE(Clear)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c) { c.clear(); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_clear()(a);
                }
        });
}

// [sequence.reqmts]/57,59: a.assign(i, j)
BOOST_AUTO_TEST_CASE(AssignIterators)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool const* first, bool const* last) { c.assign(first, last); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, a.size()}) {
                                auto const sized = alternating(k);
                                auto unsized     = unsized_alternating(k);
                                mem_assign()(a, sized.begin(), sized.end());
                                mem_assign()(a, unsized.begin(), unsized.end());
                        }
                }
        });
}

// [sequence.reqmts]/60,63: a.assign_range(rg)
BOOST_AUTO_TEST_CASE(AssignRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (T c, std::initializer_list<bool> il) { c.assign_range(il); });
#endif
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, a.size()}) {
                                mem_assign_range()(a, alternating(k));
                                mem_assign_range()(a, unsized_alternating(k));
                        }
                }
        });
}

// [sequence.reqmts]/65: a.assign(il)
BOOST_AUTO_TEST_CASE(AssignInitializerList)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, std::initializer_list<bool> il) { c.assign(il); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_assign()(a, {});
                        mem_assign()(a, {true, false, true});
                        mem_assign()(a, {true, false, true, false, true, false, true, false});
                        mem_assign()(a, {true, false, true, false, true, false, true, false, true});
                }
        });
}

// [sequence.reqmts]/66,68: a.assign(n, t)
BOOST_AUTO_TEST_CASE(AssignCount)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n, bool b) { c.assign(n, b); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, a.size()}) {
                                mem_assign()(a, k, false);
                                mem_assign()(a, k, true);
                        }
                }
        });
}

// [sequence.reqmts]/69: X(i, j), a.insert(p, i, j) and a.assign(i, j) with integers
BOOST_AUTO_TEST_CASE(IntegralArguments)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                // Two integers do not qualify as iterators, so each call takes them as a count and a value.
                if (T().max_size() >= 5UZ) {
                        BOOST_CHECK(model_of(T(2, 1)) == std::vector<bool>(2, true)); // [sequence.reqmts]/69
                        auto a = T();
                        a.assign(3, 1);
                        BOOST_CHECK(model_of(a) == std::vector<bool>(3, true)); // [sequence.reqmts]/69
                        a.insert(a.cbegin(), 2, 0);
                        BOOST_CHECK(model_of(a) == std::vector<bool>({false, false, true, true, true})); // [sequence.reqmts]/69
                }
        });
}

namespace {

// What each of the two guided sequences deduces, or cannot, from an iterator pair and one more argument.
template<class I, class... Args>
concept deduces_std_vector = requires (I i, Args... args) { std::vector(i, i, args...); };

template<class I, class... Args>
concept deduces_bit_vector = requires (I i, Args... args) { xstd::basic_bit_vector(i, i, args...); };

} // namespace

// [sequence.reqmts]/69: deduction guides
BOOST_AUTO_TEST_CASE(DeductionGuides)
{
        using I     = bool const*;
        using Alloc = std::allocator<std::size_t>;

        // A third argument that is no allocator selects no guide.
        static_assert(deduces_std_vector<I> and deduces_std_vector<I, std::allocator<bool>> and not deduces_std_vector<I, int>); // [sequence.reqmts]/69
        static_assert(std::same_as<decltype(std::vector(I(), I())), std::vector<bool>>);                                         // [sequence.reqmts]/69

        static_assert(deduces_bit_vector<I> and deduces_bit_vector<I, Alloc> and not deduces_bit_vector<I, int>); // [sequence.reqmts]/69
        static_assert(std::same_as<decltype(xstd::basic_bit_vector(I(), I(), Alloc())), xstd::bit_vector>);       // [sequence.reqmts]/69
        // An integer pair is a count and a value, whose bools name no block.
        static_assert(not deduces_bit_vector<int>); // [sequence.reqmts]/69
        BOOST_CHECK(true);
}

// [sequence.reqmts]/71,73-74: a.front()
BOOST_AUTO_TEST_CASE(Front)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) { // [sequence.reqmts]/74
                        { c.front() } -> std::same_as<typename T::reference>;
                        { cc.front() } -> std::same_as<typename T::const_reference>;
                });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_front()(a);
                }
        });
}

// [sequence.reqmts]/75,77-78: a.back()
BOOST_AUTO_TEST_CASE(Back)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc) { // [sequence.reqmts]/78
                        { c.back() } -> std::same_as<typename T::reference>;
                        { cc.back() } -> std::same_as<typename T::const_reference>;
                });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_back()(a);
                }
        });
}

// [sequence.reqmts]/84,86-88: a.emplace_back(args)
BOOST_AUTO_TEST_CASE(EmplaceBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) { // [sequence.reqmts]/88
                        { c.emplace_back(b) } -> std::same_as<typename T::reference>;
                        { c.emplace_back() } -> std::same_as<typename T::reference>;
                });
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_emplace_back()(a, false);
                        mem_emplace_back()(a, true);
                        mem_emplace_back()(a);
                }
        });
}

// [sequence.reqmts]/101,103-105,107-108: a.push_back(t), a.push_back(rv)
BOOST_AUTO_TEST_CASE(PushBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) { c.push_back(b); });                   // [sequence.reqmts]/104
                static_assert(requires (T c) { c.push_back(static_cast<bool>(c.size())); }); // [sequence.reqmts]/108
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_push_back()(a, false);
                        mem_push_back()(a, true);
                }
        });
}

// [sequence.reqmts]/109,111-112: a.append_range(rg)
BOOST_AUTO_TEST_CASE(AppendRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
#ifdef __cpp_lib_containers_ranges
                static_assert(requires (T c, std::initializer_list<bool> il) { c.append_range(il); }); // [sequence.reqmts]/112
#endif
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, 33UZ}) {
                                mem_append_range()(a, alternating(k));
                                mem_append_range()(a, unsized_alternating(k));
                        }
                }
        });
}

// [sequence.reqmts]/117,119-120: a.pop_back()
BOOST_AUTO_TEST_CASE(PopBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                static_assert(requires (T c) { c.pop_back(); }); // [sequence.reqmts]/120
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_pop_back()(a);
                }
        });
}

// [sequence.reqmts]/121,123-128: a[n], a.at(n)
BOOST_AUTO_TEST_CASE(Subscript)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
                static_assert(requires (T c, T const cc, T::size_type n) { // [sequence.reqmts]/124,128
                        { c[n] } -> std::same_as<typename T::reference>;
                        { cc[n] } -> std::same_as<typename T::const_reference>;
                        { c.at(n) } -> std::same_as<typename T::reference>;
                        { cc.at(n) } -> std::same_as<typename T::const_reference>;
                });
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        mem_subscript()(a);
                        mem_subscript()(a, i);
                        mem_at()(a, i);
                        mem_at()(a, a.size());
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
