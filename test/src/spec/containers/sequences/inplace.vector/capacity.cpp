//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_widths
#include <test/sequence/primitives.hpp> // mem_resize
#include <test/spec/random.hpp>         // all_sequences
#include <test/spec/sequence.hpp>       // inplace_vector_boundary_widths, inplace_vector_every_width, inplace_vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <concepts>                     // same_as
#include <new>                          // bad_alloc
#include <ranges>                       // iota

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Capacity)

using namespace test::sequence;

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

// [inplace.vector.capacity]/1: capacity() and max_size()
BOOST_AUTO_TEST_SUITE(CapacityMember)

// The capacity is the type's, so both answer without an object.
BOOST_AUTO_TEST_CASE_TEMPLATE(IsTheCapacityInTheType, T, test::spec::sequence::inplace_vector_every_width)
{
        static_assert(std::same_as<decltype(T::capacity()), typename T::size_type>);
        static_assert(T::capacity() == T::max_size()); // [inplace.vector.capacity]/1
        BOOST_CHECK_EQUAL(T().max_size(), T::capacity());
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.capacity]/2-7: resize(sz) and resize(sz, c)
BOOST_AUTO_TEST_SUITE(Resize)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndThrowsPastTheCapacity, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const n : std::views::iota(0UZ, T::capacity() + 1UZ)) {
                        mem_resize()(a, n);
                        mem_resize()(a, n, true);
                }
                check_resize_past_capacity(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndThrowsPastTheCapacity, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                for (auto const n : {0UZ, a.size() / 2UZ, a.size()}) {
                        mem_resize()(a, n);
                        mem_resize()(a, n, true);
                }
                check_resize_past_capacity(a);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.capacity]/8-9: reserve(n)
BOOST_AUTO_TEST_SUITE(Reserve)

BOOST_AUTO_TEST_CASE_TEMPLATE(DoesNothingUpToTheCapacityAndThrowsPastIt, T, test::spec::sequence::inplace_vector_every_width)
{
        T::reserve(0UZ);
        T::reserve(T::capacity());                                          // [inplace.vector.capacity]/8
        BOOST_CHECK_THROW(T::reserve(T::capacity() + 1UZ), std::bad_alloc); // [inplace.vector.capacity]/9
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.capacity]/10: shrink_to_fit()
BOOST_AUTO_TEST_SUITE(ShrinkToFit)

BOOST_AUTO_TEST_CASE_TEMPLATE(DoesNothing, T, test::spec::sequence::inplace_vector_every_width)
{
        auto const a = T(T::capacity(), true);
        T::shrink_to_fit(); // [inplace.vector.capacity]/10
        BOOST_CHECK(a == T(T::capacity(), true));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
