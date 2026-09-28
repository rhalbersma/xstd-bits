//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // mem_capacity, mem_reserve, mem_resize, mem_shrink_to_fit
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // sequences, vector_all
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_THROW
#include <concepts>                     // same_as
#include <stdexcept>                    // length_error

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Capacity)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

namespace {

// Nothing, half, the width, one past it, past another byte, and past the ceiling.
auto check_reserve(auto const& a)
        -> void
{
        for (auto const n : {0UZ, a.size() / 2UZ, a.size(), a.size() + 1UZ, a.size() + 9UZ, a.max_size() + 1UZ}) {
                mem_reserve()(a, n);
        }
}

// Past max_size() there is nothing to resize to, and a vector says so with length_error and no effect.
template<class X>
auto check_resize_past_max_size()
        -> void
{
        auto b = X();
        BOOST_CHECK_THROW(b.resize(b.max_size() + 1UZ), std::length_error);
        BOOST_CHECK_THROW(b.resize(b.max_size() + 1UZ, true), std::length_error);
        BOOST_CHECK(b.empty()); // [vector.capacity]/19
}

// Nothing, half, the width, one past it, and past two bytes, each with either value to fill in.
auto check_resize(auto const& a)
        -> void
{
        for (auto const n : {0UZ, a.size() / 2UZ, a.size(), a.size() + 1UZ, a.size() + 17UZ}) {
                mem_resize()(a, n);
                mem_resize()(a, n, false);
                mem_resize()(a, n, true);
        }
}

} // namespace

// [vector.capacity]/1-2: constexpr size_type capacity() const noexcept;
BOOST_AUTO_TEST_CASE(Capacity)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T const cc) { { cc.capacity() } -> std::same_as<typename T::size_type>; });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_capacity()(a);
                }
        });
}

// [vector.capacity]/3-7: constexpr void reserve(size_type n);
BOOST_AUTO_TEST_CASE(Reserve)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n) { c.reserve(n); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        check_reserve(a);
                }
        });
}

// [vector.capacity]/8-11: constexpr void shrink_to_fit();
BOOST_AUTO_TEST_CASE(ShrinkToFit)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c) { c.shrink_to_fit(); });
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        mem_shrink_to_fit()(a);
                }
        });
}

// [vector.capacity]/14-19: constexpr void resize(size_type sz); constexpr void resize(size_type sz, const T& c);
BOOST_AUTO_TEST_CASE(Resize)
{
        test::for_each_type<test::spec::sequence::vector_all>([]<class T> -> void {
                static_assert(requires (T c, T::size_type n, bool b) {
                        c.resize(n);
                        c.resize(n, b);
                });
                check_resize_past_max_size<T>();
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        check_resize(a);
                }
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
