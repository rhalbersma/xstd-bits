//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // L2, all_prefix_sequences, all_singleton_sequences, all_widths
#include <test/sequence/factory.hpp>    // limit_v
#include <test/sequence/primitives.hpp> // fn_swap_reference, mem_flip, mem_reference_assign, mem_reference_flip
#include <test/set/primitives.hpp>      // op_hash
#include <test/spec/random.hpp>         // all_sequence_key_pairs, all_sequence_pairs, all_sequences
#include <test/spec/sequence.hpp>       // vector_boundary_widths, vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                      // size_t
#include <ranges>                       // iota

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(VectorBool)
BOOST_AUTO_TEST_SUITE(Pspc)

using namespace test::sequence;

namespace {

// Every pair of positions at every width up to the quadratic limit.
template<class X>
auto every_position_pair(auto fun)
        -> void
{
        on1::all_widths<X, limit_v<X, L2>>([&](auto const& a) {
                for (auto const i : std::views::iota(0UZ, a.size())) {
                        for (auto const j : std::views::iota(0UZ, a.size())) {
                                fun(a, i, j);
                        }
                }
        });
}

} // namespace

// [vector.bool.pspc]/7-9: reference::operator= and reference::operator bool
BOOST_AUTO_TEST_SUITE(ReferenceAssignment)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryPositionPairAtBoundaryWidths, T, test::spec::sequence::vector_boundary_widths)
{
        every_position_pair<T>(mem_reference_assign());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t i) {
                mem_reference_assign()(a, i, a.size() - 1UZ - i);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.bool.pspc]/10: reference::flip()
BOOST_AUTO_TEST_SUITE(ReferenceFlip)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsAtEveryWidthAndPosition, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_widths<T>([](auto const& a) {
                for (auto const i : std::views::iota(0UZ, a.size())) {
                        mem_reference_flip()(a, i);
                }
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(mem_reference_flip());
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.bool.pspc]/11: swap(reference, reference), and with a bool on either side
BOOST_AUTO_TEST_SUITE(ReferenceSwap)

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEveryPositionPairAtBoundaryWidths, T, test::spec::sequence::vector_boundary_widths)
{
        every_position_pair<T>(fn_swap_reference());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencesAndPositions, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>([](auto const& a, std::size_t i) {
                fn_swap_reference()(a, i, a.size() - 1UZ - i);
        });
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.bool.pspc]/12: flip()
BOOST_AUTO_TEST_SUITE(Flip)

BOOST_AUTO_TEST_CASE_TEMPLATE(ComplementsEveryPositionAtBoundaryWidths, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_sequences<T>(mem_flip());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ComplementsEveryPositionOverRandomSequences, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>(mem_flip());
}

BOOST_AUTO_TEST_SUITE_END()

// [vector.bool.pspc]/13: hash<vector<bool, Allocator>>
BOOST_AUTO_TEST_SUITE(Hash)

// Equal values hash equal, whatever the capacity or allocation behind them.
BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverEverySequenceAndWidthPairAtBoundaryWidths, T, test::spec::sequence::vector_boundary_widths)
{
        on1::all_sequences<T>(test::set::op_hash());
        on1::all_widths<T, limit_v<T, L2>>([](auto const& a) {
                on1::all_widths<T, limit_v<T, L2>>([&](auto const& b) {
                        test::set::op_hash()(a, b);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldsOverRandomSequencePairs, T, test::spec::sequence::vector_random_widths)
{
        test::spec::random::all_sequences<T>(test::set::op_hash());
        test::spec::random::all_sequence_pairs<T>(test::set::op_hash());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
