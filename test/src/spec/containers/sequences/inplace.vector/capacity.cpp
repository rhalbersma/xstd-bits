//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/for_each_type.hpp>       // for_each_type
#include <test/sequence/primitives.hpp> // mem_resize
#include <test/spec/input.hpp>          // context
#include <test/spec/sequence.hpp>       // inplace_vector_all, sequences
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <concepts>                     // same_as
#include <new>                          // bad_alloc

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Capacity)

using namespace test::sequence;
using test::spec::context;
namespace inputs = test::spec::sequence::inputs;

namespace {

// One past the capacity throws, as [inplace.vector.overview]/4 has it, and changes nothing.
template<class X>
auto check_resize_past_capacity(X const& a)
        -> void
{
        auto b = a;
        BOOST_CHECK_THROW(b.resize(X::capacity() + 1UZ), std::bad_alloc);
        BOOST_CHECK_THROW(b.resize(X::capacity() + 1UZ, true), std::bad_alloc);
        BOOST_CHECK(b == a); // [inplace.vector.capacity]/4
}

} // namespace

// [inplace.vector.capacity]/1: static constexpr size_type capacity() noexcept; static size_type max_size() noexcept;
BOOST_AUTO_TEST_CASE(Capacity)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                // The capacity is the type's, so both answer without an object.
                static_assert(std::same_as<decltype(T::capacity()), typename T::size_type>);
                static_assert(T::capacity() == T::max_size()); // [inplace.vector.capacity]/1
                BOOST_CHECK_EQUAL(T().max_size(), T::capacity());
        });
}

// [inplace.vector.capacity]/2-7: constexpr void resize(size_type sz); constexpr void resize(size_type sz, const T& c);
BOOST_AUTO_TEST_CASE(Resize)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                for (auto const [from, a] : inputs::sequences<T>()) {
                        auto const on_failure = context(from, a);
                        for (auto const n : {0UZ, a.size() / 2UZ, a.size(), (a.size() + T::capacity()) / 2UZ, T::capacity()}) {
                                mem_resize()(a, n);
                                mem_resize()(a, n, true);
                        }
                        check_resize_past_capacity(a);
                }
        });
}

// [inplace.vector.capacity]/8-9: static constexpr void reserve(size_type n);
BOOST_AUTO_TEST_CASE(Reserve)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                T::reserve(0UZ);
                T::reserve(T::capacity());                                          // [inplace.vector.capacity]/8
                BOOST_CHECK_THROW(T::reserve(T::capacity() + 1UZ), std::bad_alloc); // [inplace.vector.capacity]/9
        });
}

// [inplace.vector.capacity]/10: static constexpr void shrink_to_fit() noexcept;
BOOST_AUTO_TEST_CASE(ShrinkToFit)
{
        test::for_each_type<test::spec::sequence::inplace_vector_all>([]<class T> -> void {
                auto const a = T(T::capacity(), true);
                T::shrink_to_fit(); // [inplace.vector.capacity]/10
                BOOST_CHECK(a == T(T::capacity(), true));
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
