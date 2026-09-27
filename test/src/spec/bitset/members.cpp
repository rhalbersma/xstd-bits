//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_cardinality_sets, all_singleton_set_pairs, all_singleton_sets, all_valid, any_value, empty_set, empty_set_pair, full_set
#include <test/bitset/primitives.hpp> // mem_all, mem_any, mem_at, mem_bit_and_assign, mem_bit_not, mem_bit_or_assign, mem_bit_xor_assign, mem_count, mem_equal_to, mem_flip, mem_none, mem_reset, mem_set, mem_shift_left, mem_shift_left_assign, mem_shift_right, mem_shift_right_assign, mem_size, mem_test, mem_to_string, mem_to_ullong, mem_to_ulong
#include <test/spec/bitset.hpp>       // boundary_widths, byte_widths, every_width, random_widths
#include <test/spec/random.hpp>       // all_bitset_key_pairs, all_bitset_pairs, all_bitsets
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <cstddef>                    // size_t

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(Members)

using namespace test::bitset;

namespace {

// A mutating primitive applied to a copy, so the sample it was drawn from is there for the next.
auto on_copy(auto fun, auto const& a, auto const&... args)
        -> void
{
        auto x = a;
        fun(x, args...);
}

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(TheCompoundBitwiseAssignmentsHoldOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_bit_and_assign());
        on0::empty_set_pair<T>(mem_bit_or_assign());
        on0::empty_set_pair<T>(mem_bit_xor_assign());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EqualityHoldsOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_equal_to());
}

// A width past the integer's digits overflows when full, and a run-time width is swept past both integers'.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheConversionsHoldOnTheEmptyAndFullSets, T, test::spec::bitset::every_width)
{
        on0::empty_set<T>([](auto const& bs0) {
                mem_to_ulong()(bs0);
                mem_to_ullong()(bs0);
                mem_to_string()(bs0);
        });
        on0::full_set<T>([](auto const& bsN) {
                mem_to_ulong()(bsN);
                mem_to_ullong()(bsN);
                mem_to_string()(bsN);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SetResetComplementAndFlipHoldOverEveryPosition, T, test::spec::bitset::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_set());
        on1::all_singleton_sets<T>(mem_set());
        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto& bs0) {
                        mem_set()(bs0, pos);
                });
                on0::empty_set<T>([&](auto& bs0) {
                        mem_set()(bs0, pos, true);
                });
                on0::full_set<T>([&](auto& bsN) {
                        mem_set()(bsN, pos, false);
                });
        });

        on1::all_cardinality_sets<T>(mem_reset());
        on1::all_singleton_sets<T>(mem_reset());
        on1::any_value<T>([](auto pos) {
                on0::full_set<T>([&](auto& bsN) {
                        mem_reset()(bsN, pos);
                });
        });

        on1::all_cardinality_sets<T>(mem_bit_not());
        on1::all_singleton_sets<T>(mem_bit_not());

        on1::all_cardinality_sets<T>(mem_flip());
        on1::all_singleton_sets<T>(mem_flip());
        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto& bs0) {
                        mem_flip()(bs0, pos);
                });
                on0::full_set<T>([&](auto& bsN) {
                        mem_flip()(bsN, pos);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubscriptIsTestAtEveryValidPosition, T, test::spec::bitset::boundary_widths)
{
        on1::all_valid<T>([](auto pos) {
                on0::empty_set<T>([&](auto const& bs0) {
                        mem_at()(bs0, pos);
                });
                on0::full_set<T>([&](auto const& bsN) {
                        mem_at()(bsN, pos);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheObserversHoldOverEveryCardinalityAndSingleton, T, test::spec::bitset::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_count());

        on1::all_cardinality_sets<T>(mem_size());
        on1::all_singleton_sets<T>(mem_size());

        on1::any_value<T>([](auto pos) {
                on0::empty_set<T>([&](auto const& bs0) {
                        mem_test()(bs0, pos);
                });
                on0::full_set<T>([&](auto const& bsN) {
                        mem_test()(bsN, pos);
                });
        });

        on1::all_cardinality_sets<T>(mem_all());
        on1::all_cardinality_sets<T>(mem_any());
        on1::all_cardinality_sets<T>(mem_none());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheConversionsHoldOverEveryCardinalityAndSingleton, T, test::spec::bitset::boundary_widths)
{
        on1::all_cardinality_sets<T>(mem_to_ulong());
        on1::all_cardinality_sets<T>(mem_to_ullong());
        on1::all_cardinality_sets<T>(mem_to_string());
        on1::all_singleton_sets<T>(mem_to_ulong());
        on1::all_singleton_sets<T>(mem_to_ullong());
        on1::all_singleton_sets<T>(mem_to_string());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheCompoundAssignmentsAndShiftsHoldOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_bit_and_assign());
        on2::all_singleton_set_pairs<T>(mem_bit_or_assign());
        on2::all_singleton_set_pairs<T>(mem_bit_xor_assign());

        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto& bs1) {
                        mem_shift_left_assign()(bs1, pos);
                });
        });
        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto& bs1) {
                        mem_shift_right_assign()(bs1, pos);
                });
        });

        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        mem_shift_left()(bs1, pos);
                });
        });
        on1::any_value<T>([](auto pos) {
                on1::all_singleton_sets<T>([&](auto const& bs1) {
                        mem_shift_right()(bs1, pos);
                });
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EqualityHoldsOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_equal_to());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheCompoundAssignmentsAndEqualityHoldOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>([](auto const& a, auto const& b) {
                mem_equal_to()(a, b);
                on_copy(mem_bit_and_assign(), a, b);
                on_copy(mem_bit_or_assign(), a, b);
                on_copy(mem_bit_xor_assign(), a, b);
        });
}

// Each position argument is a valid one, so a random position never reaches the throwing arm.
BOOST_AUTO_TEST_CASE_TEMPLATE(ThePositionalMembersAndShiftsHoldOverRandomBitsetsAndPositions, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_key_pairs<T>([](auto const& a, std::size_t pos) {
                mem_at()(a, pos);
                mem_test()(a, pos);
                mem_shift_left()(a, pos);
                mem_shift_right()(a, pos);
                on_copy(mem_shift_left_assign(), a, pos);
                on_copy(mem_shift_right_assign(), a, pos);
                on_copy(mem_set(), a, pos);
                on_copy(mem_set(), a, pos, false);
                on_copy(mem_reset(), a, pos);
                on_copy(mem_flip(), a, pos);
        });
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheWholeBitsetMembersObserversAndConversionsHoldOverRandomBitsets, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitsets<T>([](auto const& a) {
                on_copy(mem_set(), a);
                on_copy(mem_reset(), a);
                mem_bit_not()(a);
                on_copy(mem_flip(), a);
                mem_count()(a);
                mem_size()(a);
                mem_all()(a);
                mem_any()(a);
                mem_none()(a);
                mem_to_ulong()(a);
                mem_to_ullong()(a);
                mem_to_string()(a);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
