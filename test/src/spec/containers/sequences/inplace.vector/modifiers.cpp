//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // mem_insert_past_capacity, mem_insert_unsized_past_capacity, mem_push_back_or_throw, mem_try_emplace_back, mem_unchecked_emplace_back, mem_unchecked_push_back
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // fixed_prefixes, inplace_vector_all, prefixes
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Modifiers)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [inplace.vector.modifiers]/1-3: constexpr iterator insert(const_iterator position, const T& x);
BOOST_AUTO_TEST_CASE(Insert)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // Every insertion, emplace and append_range past the capacity throws bad_alloc with no effect.
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_insert_past_capacity()(a);
                }
                // A range with no size overruns the capacity one element at a time, at every fixed width and position.
                for (auto const [from, a] : inputs::fixed_prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_insert_unsized_past_capacity()(a);
                }
        });
}

// [inplace.vector.modifiers]/4-7: constexpr reference push_back(const T& x);
BOOST_AUTO_TEST_CASE(PushBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_push_back_or_throw()(a, false);
                        mem_push_back_or_throw()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/8-14: template<class... Args> optional<reference> try_emplace_back(Args&&... args);
BOOST_AUTO_TEST_CASE(TryEmplaceBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_try_emplace_back()(a, false);
                        mem_try_emplace_back()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/15-16: template<class... Args> constexpr reference unchecked_emplace_back(Args&&... args);
BOOST_AUTO_TEST_CASE(UncheckedEmplaceBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_unchecked_emplace_back()(a, false);
                        mem_unchecked_emplace_back()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/17-18: constexpr reference unchecked_push_back(const T& x);
BOOST_AUTO_TEST_CASE(UncheckedPushBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_unchecked_push_back()(a, false);
                        mem_unchecked_push_back()(a, true);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
