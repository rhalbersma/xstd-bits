//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_sequences
#include <test/set/exhaustive.hpp>      // all_cardinality_sets
#include <test/set/primitives.hpp>      // mem_empty, mem_max_size, mem_size
#include <test/spec/random.hpp>         // all_sequences, all_sets
#include <test/spec/sequence.hpp>       // boundary_widths, random_widths
#include <test/spec/set.hpp>            // boundary_widths, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Size)

using namespace test;
using namespace test::set;

// [container.reqmts]/52-55: c.size()
BOOST_AUTO_TEST_SUITE(Size)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryCardinalitySet, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(mem_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>(mem_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>(mem_size());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/56-58: c.max_size()
BOOST_AUTO_TEST_SUITE(MaxSize)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryCardinalitySet, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_max_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(mem_max_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>(mem_max_size());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>(mem_max_size());
}

BOOST_AUTO_TEST_SUITE_END()

// [container.reqmts]/59-62: c.empty()
BOOST_AUTO_TEST_SUITE(Empty)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryCardinalitySet, T, test::spec::set::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>(mem_empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>(mem_empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>(mem_empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
