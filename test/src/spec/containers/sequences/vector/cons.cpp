//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // L1
#include <test/sequence/factory.hpp>    // model_of
#include <test/spec/sequence.hpp>       // vector_every_width
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <ranges>                       // iota
#include <vector>                       // vector

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(Vector)
BOOST_AUTO_TEST_SUITE(Cons)

using namespace test::sequence;

// [vector.cons]/1-2: vector(const Allocator&), which the default constructor delegates to
BOOST_AUTO_TEST_SUITE(DefaultConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructsAnEmptySequence, T, test::spec::sequence::vector_every_width)
{
        BOOST_CHECK(T().empty()); // [vector.cons]/1
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.cons]/3-5: vector(size_type n, const Allocator&)
BOOST_AUTO_TEST_SUITE(CountConstructor)

// n default-inserted bools, each of them false.
BOOST_AUTO_TEST_CASE_TEMPLATE(ConstructsThatManyFalsePositionsAtEveryWidth, T, test::spec::sequence::vector_every_width)
{
        for (auto const n : std::views::iota(0UZ, L1 + 1UZ)) {
                BOOST_CHECK(model_of(T(n)) == std::vector<bool>(n)); // [vector.cons]/4
        }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
