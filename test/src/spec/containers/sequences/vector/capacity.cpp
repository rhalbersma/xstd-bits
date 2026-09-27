//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_widths
#include <test/sequence/primitives.hpp> // mem_capacity, mem_reserve, mem_resize, mem_shrink_to_fit
#include <test/spec/random.hpp>         // all_sequences
#include <test/spec/sequence.hpp>       // vector_boundary_widths, vector_every_width, vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_THROW
#include <stdexcept>                    // length_error

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Capacity)

using namespace test::sequence;

namespace {

// Nothing, half, the width, one past it, past another byte, and past the ceiling.
auto check_reserve(auto const& a)
        -> void
{
        for (auto const n : {0UZ, a.size() / 2UZ, a.size(), a.size() + 1UZ, a.size() + 9UZ, a.max_size() + 1UZ}) {
                mem_reserve()(a, n);
        }
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

// [vector.capacity]/1-2: capacity()
BOOST_AUTO_TEST_SUITE(CapacityMember)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>(mem_capacity());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>(mem_capacity());
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.capacity]/3-7: reserve(n)
BOOST_AUTO_TEST_SUITE(Reserve)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                check_reserve(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                check_reserve(a);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.capacity]/8-11: shrink_to_fit()
BOOST_AUTO_TEST_SUITE(ShrinkToFit)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>(mem_shrink_to_fit());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>(mem_shrink_to_fit());
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.capacity]/14-19: resize(sz) and resize(sz, c)
BOOST_AUTO_TEST_SUITE(Resize)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidth, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                check_resize(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                check_resize(a);
        });
}

// Past max_size() there is nothing to resize to, and a vector says so with length_error and no effect.
BOOST_AUTO_TEST_CASE_TEMPLATE(ThrowsLengthErrorPastMaxSize, T, test::spec::sequence::vector_every_width)
{
        auto a = T();
        BOOST_CHECK_THROW(a.resize(a.max_size() + 1UZ), std::length_error);
        BOOST_CHECK_THROW(a.resize(a.max_size() + 1UZ, true), std::length_error);
        BOOST_CHECK(a.empty()); // [vector.capacity]/19
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
