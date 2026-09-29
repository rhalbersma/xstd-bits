//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // fn_swap_reference, mem_flip, mem_reference_assign, mem_reference_flip, ref_copy, ref_destructor, ref_operator_bool
#include <test/set/primitives.hpp>      // op_hash
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // index_pairs, indexed, pairs, sequences, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL
#include <concepts>                     // same_as
#include <cstddef>                      // size_t
#include <functional>                   // hash

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(VectorBool)
BOOST_AUTO_TEST_SUITE(Pspc)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// [vector.bool.pspc]/5: constexpr reference::reference(const reference& x) noexcept;
BOOST_AUTO_TEST_CASE(ReferenceCopy)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        ref_copy()(a, i);
                }
        });
}

// [vector.bool.pspc]/6: constexpr reference::~reference();
BOOST_AUTO_TEST_CASE(ReferenceDestructor)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        ref_destructor()(a, i);
                }
        });
}

// [vector.bool.pspc]/7-8: constexpr reference& reference::operator=(bool x) noexcept; and two more
BOOST_AUTO_TEST_CASE(ReferenceAssign)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, i, j] : inputs::index_pairs<T>()) {
                        auto const on_failure = context(from, a, i, j);
                        mem_reference_assign()(a, i, j);
                }
        });
}

// [vector.bool.pspc]/9: constexpr reference::operator bool() const noexcept;
BOOST_AUTO_TEST_CASE(ReferenceBool)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        ref_operator_bool()(a, i);
                }
        });
}

// [vector.bool.pspc]/10: constexpr void reference::flip() noexcept;
BOOST_AUTO_TEST_CASE(ReferenceFlip)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n) { { c[n].flip() } -> std::same_as<void>; });
                for (auto const [from, a, i] : inputs::indexed<T>()) {
                        auto const on_failure = context(from, a, i);
                        mem_reference_flip()(a, i);
                }
        });
}

// [vector.bool.pspc]/11: constexpr void swap(reference x, reference y) noexcept; and the two with a bool&
BOOST_AUTO_TEST_CASE(ReferenceSwap)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n) { swap(c[n], c[n]); });
                static_assert(reference_swaps_with_bool<T> or is_std_vector_bool_v<T>); // [vector.bool.pspc]/11
                for (auto const [from, a, i, j] : inputs::index_pairs<T>()) {
                        auto const on_failure = context(from, a, i, j);
                        fn_swap_reference()(a, i, j);
                }
        });
}

// [vector.bool.pspc]/12: constexpr void flip() noexcept;
BOOST_AUTO_TEST_CASE(Flip)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c) { c.flip(); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_flip()(a);
                }
        });
}

// [vector.bool.pspc]/13: template<class Allocator> struct hash<vector<bool, Allocator>>;
BOOST_AUTO_TEST_CASE(Hash)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T const cc) { { std::hash<T>()(cc) } -> std::same_as<std::size_t>; });
                // Equal values hash equal, whatever the capacity or allocation behind them.
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        test::set::op_hash()(a);
                        BOOST_CHECK_EQUAL(std::hash<T>()(a), std::hash<T>()(T(a))); // [vector.bool.pspc]/13
                }
                for (auto const [from, a, b] : inputs::pairs<T>()) {
                        auto const on_failure = context(from, a, b);
                        test::set::op_hash()(a, b);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
