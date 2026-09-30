//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // is_std_inplace_vector_v, mem_erase_keeps_prefix, mem_insert_past_capacity, mem_insert_unsized_past_capacity, mem_push_back_or_throw, mem_try_emplace_back, mem_unchecked_emplace_back, mem_unchecked_push_back
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // fixed_prefixes, inplace_vector_all, pairs, positions, prefixes
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                     // same_as, swap
#include <optional>                     // optional

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Modifiers)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [inplace.vector.modifiers]/3: constexpr iterator insert(const_iterator position, const T& x);
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

// [inplace.vector.modifiers]/4-5,7: constexpr reference push_back(const T& x);
BOOST_AUTO_TEST_CASE(PushBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) { { c.push_back(b) } -> std::same_as<typename T::reference>; });
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_push_back_or_throw()(a, false);
                        mem_push_back_or_throw()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/10-12: template<class... Args> optional<reference> try_emplace_back(Args&&... args);
BOOST_AUTO_TEST_CASE(TryEmplaceBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) {
                        c.try_emplace_back(b);
                        c.try_push_back(b);
                });

                // P3981R0's return type, held to the packing alone: libstdc++ 16 still returns P0843R14's pointer.
                static_assert(is_std_inplace_vector_v<T> or requires (T c, bool b) {
                        { c.try_emplace_back(b) } -> std::same_as<std::optional<typename T::reference>>;
                        { c.try_push_back(b) } -> std::same_as<std::optional<typename T::reference>>;
                });

                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_try_emplace_back()(a, false);
                        mem_try_emplace_back()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/16: template<class... Args> constexpr reference unchecked_emplace_back(Args&&... args);
BOOST_AUTO_TEST_CASE(UncheckedEmplaceBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) { { c.unchecked_emplace_back(b) } -> std::same_as<typename T::reference>; });
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_unchecked_emplace_back()(a, false);
                        mem_unchecked_emplace_back()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/18: constexpr reference unchecked_push_back(const T& x);
BOOST_AUTO_TEST_CASE(UncheckedPushBack)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(requires (T c, bool b) { { c.unchecked_push_back(b) } -> std::same_as<typename T::reference>; });
                for (auto const [from, a] : inputs::prefixes<T>()) {
                        auto const on_failure = context(from, a);
                        mem_unchecked_push_back()(a, false);
                        mem_unchecked_push_back()(a, true);
                }
        });
}

// [inplace.vector.modifiers]/19-20: constexpr iterator erase(const_iterator position);
BOOST_AUTO_TEST_CASE(Erase)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a, p] : inputs::positions<T>()) {
                        auto const on_failure = context(from, a, p);
                        mem_erase_keeps_prefix()(a, p);
                }
        });
}

// [inplace.vector.modifiers]/23: constexpr void swap(inplace_vector& x) noexcept(N == 0 || ...);
BOOST_AUTO_TEST_CASE(Swap)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(requires (T x, T y) {
                        { x.swap(y) } noexcept;
                        { swap(x, y) } noexcept;
                        { std::ranges::swap(x, y) } noexcept;
                }); // [inplace.vector.modifiers]/23
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        auto x = a;
                        auto y = b;
                        x.swap(y);
                        BOOST_CHECK(x == b and y == a); // [inplace.vector.modifiers]/23
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
