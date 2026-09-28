//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_prefix_sequences, all_widths
#include <test/sequence/primitives.hpp> // fn_format
#include <test/spec/random.hpp>         // all_sequences
#include <test/spec/sequence.hpp>       // vector_boundary_widths, vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(VectorBool)
BOOST_AUTO_TEST_SUITE(Fmt)

using namespace test::sequence;

// [vector.bool.fmt]/1-2: parse and format, each the underlying bool formatter's
BOOST_AUTO_TEST_SUITE(Format)

BOOST_AUTO_TEST_CASE_TEMPLATE(FormatsAsTheBoolItStandsForAtEveryWidthAndPrefix, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>(fn_format());
        on1::all_prefix_sequences<T>(fn_format());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FormatsAsTheBoolItStandsForOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>(fn_format());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
