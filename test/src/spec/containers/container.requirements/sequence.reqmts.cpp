//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/factory.hpp>    // model_of
#include <test/sequence/primitives.hpp> // alternating, constructor, mem_append_range, mem_assign, mem_assign_range, mem_at, mem_back, mem_clear, mem_emplace, mem_emplace_back, mem_erase, mem_front, mem_insert, mem_insert_range, mem_pop_back, mem_push_back, mem_subscript, op_assign, unsized_alternating
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // all, growable_all, indexed, positions, prefixes, sequences, spans
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <ranges>                       // from_range

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(SequenceReqmts)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [sequence.reqmts]/5-7: X u(n, t);
BOOST_AUTO_TEST_CASE(CountConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        constructor<T>()(a.size(), false);
                        constructor<T>()(a.size(), true);
                }
        });
}

// [sequence.reqmts]/8-10: X u(i, j);
BOOST_AUTO_TEST_CASE(IteratorConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        auto const sized = model_of(a);
                        auto unsized = unsized_alternating(a.size());
                        constructor<T>()(sized.begin(), sized.end());
                        constructor<T>()(unsized.begin(), unsized.end());
                }
        });
}

// [sequence.reqmts]/11-14: X(from_range, rg)
BOOST_AUTO_TEST_CASE(RangeConstructor)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
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
                constructor<T>()({});
                constructor<T>()({true});
                constructor<T>()({true, false, true});
                constructor<T>()({true, false, true, false, true, false, true, false});
                constructor<T>()({true, false, true, false, true, false, true, false, true});
        });
}

// [sequence.reqmts]/16-19: a = il
BOOST_AUTO_TEST_CASE(InitializerListAssignment)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        op_assign()(a, {});
                        op_assign()(a, {false, true});
                }
        });
}

// [sequence.reqmts]/20-23: a.emplace(p, args)
BOOST_AUTO_TEST_CASE(Emplace)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_emplace()(a, p, false);
                        mem_emplace()(a, p, true);
                        mem_emplace()(a, p);
                }
        });
}

// [sequence.reqmts]/24-31: a.insert(p, t), a.insert(p, rv)
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert()(a, p, false);
                        mem_insert()(a, p, true);
                }
        });
}

// [sequence.reqmts]/32-35: a.insert(p, n, t)
BOOST_AUTO_TEST_CASE(InsertCount)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        mem_insert()(a, p, n, true);
                }
        });
}

// [sequence.reqmts]/36-39: a.insert(p, i, j)
BOOST_AUTO_TEST_CASE(InsertIterators)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        auto const sized = alternating(n);
                        auto unsized = unsized_alternating(n);
                        mem_insert()(a, p, sized.begin(), sized.end());
                        mem_insert()(a, p, unsized.begin(), unsized.end());
                }
        });
}

// [sequence.reqmts]/40-43: a.insert_range(p, rg)
BOOST_AUTO_TEST_CASE(InsertRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
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
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_insert()(a, p, {true, false, true});
                        mem_insert()(a, p, {true, false, true, false, true, false, true, false});
                        mem_insert()(a, p, {true, false, true, false, true, false, true, false, true});
                }
        });
}

// [sequence.reqmts]/45-48: a.erase(q)
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_erase()(a, p);
                }
        });
}

// [sequence.reqmts]/49-52: a.erase(q1, q2)
BOOST_AUTO_TEST_CASE(EraseRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a, p, n] : inputs::spans<T>()) {
                        auto const on_failure = context(from, a, p, n);
                        mem_erase()(a, p, p + n);
                }
        });
}

// [sequence.reqmts]/53-56: a.clear()
BOOST_AUTO_TEST_CASE(Clear)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_clear()(a);
                }
        });
}

// [sequence.reqmts]/57-59: a.assign(i, j)
BOOST_AUTO_TEST_CASE(AssignIterators)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, a.size()}) {
                                auto const sized = alternating(k);
                                auto unsized = unsized_alternating(k);
                                mem_assign()(a, sized.begin(), sized.end());
                                mem_assign()(a, unsized.begin(), unsized.end());
                        }
                }
        });
}

// [sequence.reqmts]/60-64: a.assign_range(rg)
BOOST_AUTO_TEST_CASE(AssignRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
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
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_assign()(a, {});
                        mem_assign()(a, {true, false, true});
                        mem_assign()(a, {true, false, true, false, true, false, true, false});
                        mem_assign()(a, {true, false, true, false, true, false, true, false, true});
                }
        });
}

// [sequence.reqmts]/66-68: a.assign(n, t)
BOOST_AUTO_TEST_CASE(AssignCount)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, a.size()}) {
                                mem_assign()(a, k, false);
                                mem_assign()(a, k, true);
                        }
                }
        });
}

// [sequence.reqmts]/71-74: a.front()
BOOST_AUTO_TEST_CASE(Front)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_front()(a);
                }
        });
}

// [sequence.reqmts]/75-78: a.back()
BOOST_AUTO_TEST_CASE(Back)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_back()(a);
                }
        });
}

// [sequence.reqmts]/84-88: a.emplace_back(args)
BOOST_AUTO_TEST_CASE(EmplaceBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_emplace_back()(a, false);
                        mem_emplace_back()(a, true);
                        mem_emplace_back()(a);
                }
        });
}

// [sequence.reqmts]/101-108: a.push_back(t), a.push_back(rv)
BOOST_AUTO_TEST_CASE(PushBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_push_back()(a, false);
                        mem_push_back()(a, true);
                }
        });
}

// [sequence.reqmts]/109-112: a.append_range(rg)
BOOST_AUTO_TEST_CASE(AppendRange)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const k : {0UZ, 1UZ, 7UZ, 8UZ, 9UZ, 17UZ, 33UZ}) {
                                mem_append_range()(a, alternating(k));
                                mem_append_range()(a, unsized_alternating(k));
                        }
                }
        });
}

// [sequence.reqmts]/117-120: a.pop_back()
BOOST_AUTO_TEST_CASE(PopBack)
{
        test::for_each_type<test::spec::sequence::growable_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_pop_back()(a);
                }
        });
}

// [sequence.reqmts]/121-128: a[n], a.at(n)
BOOST_AUTO_TEST_CASE(Subscript)
{
        test::for_each_type<test::spec::sequence::all>([]<class T> -> void {
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
