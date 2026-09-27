//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/exhaustive.hpp> // all_widths, full_sequence
#include <test/sequence/primitives.hpp> // mem_insert_past_capacity, mem_push_back_or_throw, mem_try_emplace_back, mem_unchecked_emplace_back, mem_unchecked_push_back
#include <test/spec/random.hpp>         // all_sequence_key_pairs, all_sequences
#include <test/spec/sequence.hpp>       // inplace_vector_boundary_widths, inplace_vector_every_width, inplace_vector_random_widths
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                      // size_t
#include <type_traits>                  // remove_cvref_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Containers)
BOOST_AUTO_TEST_SUITE(Sequences)
BOOST_AUTO_TEST_SUITE(InplaceVector)
BOOST_AUTO_TEST_SUITE(Modifiers)

using namespace test::sequence;

namespace {

// Either value appended.
auto both_values(auto fun)
{
        return [fun](auto const& a) {
                fun(a, false);
                fun(a, true);
        };
}

// The first p positions of a random sequence, which a sequence drawn at its capacity needs to have room at all.
auto on_prefix(auto fun)
{
        return [fun](auto const& a, std::size_t p) {
                using X = std::remove_cvref_t<decltype(a)>;
                fun(X(a.begin(), a.begin() + static_cast<X::difference_type>(p)));
        };
}

} // namespace

// [inplace.vector.modifiers]/1-3: insert, insert_range, emplace and append_range
BOOST_AUTO_TEST_SUITE(Insert)

BOOST_AUTO_TEST_CASE_TEMPLATE(ThrowsBadAllocWithNoEffectOnAFullSequence, T, test::spec::sequence::inplace_vector_every_width)
{
        on0::full_sequence<T>(mem_insert_past_capacity());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ThrowsBadAllocWithNoEffectPastTheCapacityAtEveryWidth, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>(mem_insert_past_capacity());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ThrowsBadAllocWithNoEffectPastTheCapacityOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequences<T>(mem_insert_past_capacity());
        test::spec::random::all_sequence_key_pairs<T>(on_prefix(mem_insert_past_capacity()));
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.modifiers]/4-7: push_back and emplace_back
BOOST_AUTO_TEST_SUITE(PushBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(ThrowsBadAllocWithNoEffectOnAFullSequence, T, test::spec::sequence::inplace_vector_every_width)
{
        on0::full_sequence<T>(both_values(mem_push_back_or_throw()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackAtEveryWidth, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>(both_values(mem_push_back_or_throw()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(on_prefix(both_values(mem_push_back_or_throw())));
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.modifiers]/8-14: try_emplace_back and try_push_back
BOOST_AUTO_TEST_SUITE(TryEmplaceBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(AnswersDisengagedWithNoEffectOnAFullSequence, T, test::spec::sequence::inplace_vector_every_width)
{
        on0::full_sequence<T>(both_values(mem_try_emplace_back()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AnswersTheNewBackAtEveryWidth, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>(both_values(mem_try_emplace_back()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AnswersTheNewBackOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(on_prefix(both_values(mem_try_emplace_back())));
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.modifiers]/15-16: unchecked_emplace_back
BOOST_AUTO_TEST_SUITE(UncheckedEmplaceBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackAtEveryWidth, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>(both_values(mem_unchecked_emplace_back()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(on_prefix(both_values(mem_unchecked_emplace_back())));
}

BOOST_AUTO_TEST_SUITE_END()

// [inplace.vector.modifiers]/17-18: unchecked_push_back
BOOST_AUTO_TEST_SUITE(UncheckedPushBack)

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackAtEveryWidth, T, test::spec::sequence::inplace_vector_boundary_widths)
{
        on1::all_widths<T>(both_values(mem_unchecked_push_back()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ReturnsTheNewBackOverRandomSequences, T, test::spec::sequence::inplace_vector_random_widths)
{
        test::spec::random::all_sequence_key_pairs<T>(on_prefix(both_values(mem_unchecked_push_back())));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
