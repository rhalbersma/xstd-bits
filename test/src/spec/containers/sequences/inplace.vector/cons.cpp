//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/factory.hpp>    // model_of
#include <test/sequence/primitives.hpp> // alternating
#include <test/spec/sequence.hpp>       // inplace_vector_every_width
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_THROW
#include <new>                          // bad_alloc
#include <ranges>                       // from_range, iota
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;

// Past the capacity every constructor throws bad_alloc, as [inplace.vector.overview]/4 has it of any member that grows.

// [inplace.vector.cons]/1-3: inplace_vector(size_type n)
BOOST_AUTO_TEST_SUITE(CountConstructor)

// n default-inserted bools, each of them false.
BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthUpToTheCapacityAndThrowsPastIt, T, test::spec::sequence::inplace_vector_every_width)
{
        for (auto const n : std::views::iota(0UZ, T::capacity() + 1UZ)) {
                BOOST_CHECK(model_of(T(n)) == std::vector<bool>(n)); // [inplace.vector.cons]/2
        }
        BOOST_CHECK_THROW(static_cast<void>(T(T::capacity() + 1UZ)), std::bad_alloc);
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.cons]/4-6: inplace_vector(size_type n, const T& value)
BOOST_AUTO_TEST_SUITE(CountCopiesConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthUpToTheCapacityAndThrowsPastIt, T, test::spec::sequence::inplace_vector_every_width)
{
        for (auto const n : std::views::iota(0UZ, T::capacity() + 1UZ)) {
                BOOST_CHECK(model_of(T(n, true)) == std::vector<bool>(n, true)); // [inplace.vector.cons]/5
        }
        BOOST_CHECK_THROW(static_cast<void>(T(T::capacity() + 1UZ, true)), std::bad_alloc);
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.cons]/7-8: inplace_vector(InputIterator first, InputIterator last)
BOOST_AUTO_TEST_SUITE(IteratorConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthUpToTheCapacityAndThrowsPastIt, T, test::spec::sequence::inplace_vector_every_width)
{
        for (auto const n : std::views::iota(0UZ, T::capacity() + 1UZ)) {
                auto const in = alternating(n);
                BOOST_CHECK(model_of(T(in.begin(), in.end())) == in); // [inplace.vector.cons]/7
        }
        auto const more = alternating(T::capacity() + 1UZ);
        BOOST_CHECK_THROW(static_cast<void>(T(more.begin(), more.end())), std::bad_alloc);
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.cons]/9-11: inplace_vector(from_range_t, R&& rg)
BOOST_AUTO_TEST_SUITE(RangeConstructor)

// The standard libraries without P1206R7 have no from_range constructor to check.
BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthUpToTheCapacityAndThrowsPastIt, T, test::spec::sequence::inplace_vector_every_width)
{
        auto const more = alternating(T::capacity() + 1UZ);
        if constexpr (requires { T(std::from_range, more); }) {
                for (auto const n : std::views::iota(0UZ, T::capacity() + 1UZ)) {
                        auto const in = alternating(n);
                        BOOST_CHECK(model_of(T(std::from_range, in)) == in); // [inplace.vector.cons]/10
                }
                BOOST_CHECK_THROW(static_cast<void>(T(std::from_range, more)), std::bad_alloc);
        }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
