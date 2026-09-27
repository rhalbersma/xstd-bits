//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_doubleton_set_pairs, all_singleton_set_pairs, empty_set_pair
#include <test/bitset/primitives.hpp> // mem_is_proper_subset_of, mem_is_proper_subset_of_edges, mem_is_subset_of
#include <test/spec/bitset.hpp>       // byte_widths, every_width, few_widths, random_widths
#include <test/spec/random.hpp>       // all_bitset_pairs, all_bitsets
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Boost)
BOOST_AUTO_TEST_SUITE(DynamicBitset)
BOOST_AUTO_TEST_SUITE(Subset)

using namespace test::bitset;

// std::bitset has no subset test, so it is answered position by position, which is what the member is checked against.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubsetTestsHoldOnAnEmptyPair, T, test::spec::bitset::every_width)
{
        on0::empty_set_pair<T>(mem_is_subset_of());
        on0::empty_set_pair<T>(mem_is_proper_subset_of());
        on0::empty_set_pair<T>(mem_is_proper_subset_of_edges());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubsetTestsHoldOverEverySingletonPair, T, test::spec::bitset::byte_widths)
{
        on2::all_singleton_set_pairs<T>(mem_is_subset_of());
        on2::all_singleton_set_pairs<T>(mem_is_proper_subset_of());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubsetTestsHoldOverEveryDoubletonPair, T, test::spec::bitset::few_widths)
{
        on4::all_doubleton_set_pairs<T>(mem_is_subset_of());
        on4::all_doubleton_set_pairs<T>(mem_is_proper_subset_of());
}

// A random bitset is also the base of the four edges, which then differ in its first and last blocks.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubsetTestsHoldOverRandomPairs, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitset_pairs<T>([](auto const& a, auto const& b) {
                mem_is_subset_of()(a, b);
                mem_is_proper_subset_of()(a, b);
        });
        test::spec::random::all_bitsets<T>([](auto& a) {
                mem_is_proper_subset_of_edges()(a, a);
        });
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
