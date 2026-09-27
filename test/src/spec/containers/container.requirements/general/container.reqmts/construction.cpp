//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_sequence_pairs, all_sequences
#include <test/sequence/primitives.hpp> // constructor_copy, constructor_default, constructor_move, op_copy_assign, op_move_assign
#include <test/spec/random.hpp>         // all_sequence_pairs, all_sequences
#include <test/spec/sequence.hpp>       // boundary_widths, every_width, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Construction)

using namespace test;

// [container.reqmts]/10-11: X u
BOOST_AUTO_TEST_SUITE(DefaultConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsForEverySequence, T, test::spec::sequence::every_width)
{
        sequence::constructor_default<T>()();
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/12-14: X u(v)
BOOST_AUTO_TEST_SUITE(CopyConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>(sequence::constructor_copy());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>(sequence::constructor_copy());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/15-16: X u(rv)
BOOST_AUTO_TEST_SUITE(MoveConstructor)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>(sequence::constructor_move());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>(sequence::constructor_move());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/17-19: t = v
BOOST_AUTO_TEST_SUITE(CopyAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequencePairAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on2::all_sequence_pairs<T>(sequence::op_copy_assign());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencePairs, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequence_pairs<T>(sequence::op_copy_assign());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/20-23: t = rv
BOOST_AUTO_TEST_SUITE(MoveAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequencePairAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on2::all_sequence_pairs<T>(sequence::op_move_assign());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencePairs, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequence_pairs<T>(sequence::op_move_assign());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
