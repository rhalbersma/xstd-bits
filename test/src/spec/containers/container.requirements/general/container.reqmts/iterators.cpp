//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_sequences
#include <test/set/exhaustive.hpp>      // all_cardinality_sets, all_singleton_sets
#include <test/set/primitives.hpp>      // mem_const_iterator
#include <test/spec/random.hpp>         // all_sequences, all_sets
#include <test/spec/sequence.hpp>       // boundary_widths, random_widths
#include <test/spec/set.hpp>            // boundary_widths, random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <utility>                      // as_const

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(ContainerRequirements)
BOOST_AUTO_TEST_SUITE(General)
BOOST_AUTO_TEST_SUITE(ContainerReqmts)
BOOST_AUTO_TEST_SUITE(Iterators)

using namespace test;
using namespace test::set;

// [container.reqmts]/27-38: b.begin(), b.end(), b.cbegin() and b.cend()
BOOST_AUTO_TEST_SUITE(Begin)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryCardinalityAndSingletonSet, T, test::spec::set::boundary_widths)
{
        auto const check = [](auto& is) {
                mem_const_iterator()(is);
                mem_const_iterator()(std::as_const(is));
        };
        on1::all_cardinality_sets<T>(check);
        on1::all_singleton_sets<T>(check);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSets, T, test::spec::set::random_widths)
{
        spec::random::all_sets<T>([](auto& is) {
                mem_const_iterator()(is);
                mem_const_iterator()(std::as_const(is));
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAtBoundaryWidths, T, test::spec::sequence::boundary_widths)
{
        sequence::on1::all_sequences<T>([](auto& a) {
                mem_const_iterator()(a);
                mem_const_iterator()(std::as_const(a));
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequences, T, test::spec::sequence::random_widths)
{
        spec::random::all_sequences<T>([](auto& a) {
                mem_const_iterator()(a);
                mem_const_iterator()(std::as_const(a));
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
