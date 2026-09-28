//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_prefix_sequences, all_singleton_sequences, all_widths
#include <test/sequence/primitives.hpp> // fn_erase, fn_erase_if
#include <test/spec/random.hpp>         // all_sequences
#include <test/spec/sequence.hpp>       // inplace_vector_boundary_widths, inplace_vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Erasure)

using namespace test::sequence;

namespace {

// Either value.
auto check_erase(auto const& a)
        -> void
{
        fn_erase()(a, false);
        fn_erase()(a, true);
}

// A predicate that keeps everything, nothing, or one of the two values.
auto check_erase_if(auto const& a)
        -> void
{
        fn_erase_if()(a, [](bool) -> bool { return false; });
        fn_erase_if()(a, [](bool) -> bool { return true; });
        fn_erase_if()(a, [](bool x) -> bool { return x; });
}

} // namespace

// [inplace.vector.erasure]/1: erase(c, value)
BOOST_AUTO_TEST_SUITE(Erase)

BOOST_AUTO_TEST_CASE_TEMPLATE(RemovesWhatItRemovesFromAStdInplaceVectorBoolAtBoundaryWidths, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_sequences<T>([](auto const& a) {
                check_erase(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(RemovesWhatItRemovesFromAStdInplaceVectorBoolOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                check_erase(a);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.erasure]/2: erase_if(c, pred)
BOOST_AUTO_TEST_SUITE(EraseIf)

BOOST_AUTO_TEST_CASE_TEMPLATE(RemovesWhatItRemovesFromAStdInplaceVectorBoolAtBoundaryWidths, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_sequences<T>([](auto const& a) {
                check_erase_if(a);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(RemovesWhatItRemovesFromAStdInplaceVectorBoolOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequences<T>([](auto const& a) {
                check_erase_if(a);
        });
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
