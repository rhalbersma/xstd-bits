//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/reference.hpp>           // proxy_reference
#include <test/sequence/primitives.hpp> // mem_flip, mem_reference_flip
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // indexed, inplace_vector_all, sequences
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                     // same_as

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Xstd)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Flip)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

// xstd inplace_vector: constexpr void flip() noexcept;
BOOST_AUTO_TEST_CASE(Flip)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // [vector.bool.pspc]'s flip comes with a proxy reference, which std::inplace_vector<bool, N> lacks.
                static_assert(test::proxy_reference<T> == requires (T c) { { c.flip() } -> std::same_as<void>; });
                if constexpr (test::proxy_reference<T>) {
                        for (auto const [from, a] : inputs::sequences<T>()) {
                                auto const on_failure = context(from, a);
                                mem_flip()(a);
                        }
                }
        });
}

// xstd inplace_vector: constexpr void reference::flip() noexcept;
BOOST_AUTO_TEST_CASE(ReferenceFlip)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                static_assert(test::proxy_reference<T> == requires (T c, T::size_type n) { { c[n].flip() } -> std::same_as<void>; });
                if constexpr (test::proxy_reference<T>) {
                        for (auto const [from, a, i] : inputs::indexed<T>()) {
                                auto const on_failure = context(from, a, i);
                                mem_reference_flip()(a, i);
                        }
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
