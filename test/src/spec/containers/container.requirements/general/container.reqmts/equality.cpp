//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_sequence_pairs, empty_sequence_pair
#include <test/set/exhaustive.hpp>      // all_singleton_set_pairs, empty_set_pair
#include <test/set/primitives.hpp>      // op_equal_to, op_not_equal_to
#include <test/spec/random.hpp>         // all_sequence_pairs, all_set_pairs
#include <test/spec/sequence.hpp>       // boundary_widths, every_width, random_widths
#include <test/spec/set.hpp>            // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Equality)

using namespace test;
using namespace test::set;

// [container.reqmts]/42-46: c == b
BOOST_AUTO_TEST_SUITE(EqualTo)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOnAnEmptySetPair, T, test::spec::set::every_width)
{
        on0::empty_set_pair<T>(op_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySingletonSetPair, T, test::spec::set::boundary_widths)
{
        on2::all_singleton_set_pairs<T>(op_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSetPairs, T, test::spec::set::random_widths)
{
        spec::random::all_set_pairs<T>(op_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOnAnEmptySequencePair, T, test::spec::sequence::every_width)
{
        sequence::on0::empty_sequence_pair<T>(op_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequencePairAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on2::all_sequence_pairs<T>(op_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencePairs, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequence_pairs<T>(op_equal_to());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/47: c != b
BOOST_AUTO_TEST_SUITE(NotEqualTo)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOnAnEmptySetPair, T, test::spec::set::every_width)
{
        on0::empty_set_pair<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySingletonSetPair, T, test::spec::set::boundary_widths)
{
        on2::all_singleton_set_pairs<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSetPairs, T, test::spec::set::random_widths)
{
        spec::random::all_set_pairs<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOnAnEmptySequencePair, T, test::spec::sequence::every_width)
{
        sequence::on0::empty_sequence_pair<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequencePairAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on2::all_sequence_pairs<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencePairs, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequence_pairs<T>(op_not_equal_to());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
