//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/bitset/exhaustive.hpp> // all_cardinality_sets, all_singleton_sets, any_value
#include <test/bitset/primitives.hpp> // constructor, string_constructor
#include <test/dynamic.hpp>           // dynamic
#include <test/spec/bitset.hpp>       // boundary_widths, every_width, random_widths
#include <test/spec/random.hpp>       // all_bitsets
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <array>                      // array

BOOST_AUTO_TEST_SUITE(Spec)
BOOST_AUTO_TEST_SUITE(Utilities)
BOOST_AUTO_TEST_SUITE(Bitset)
BOOST_AUTO_TEST_SUITE(TemplateBitset)
BOOST_AUTO_TEST_SUITE(BitsetCons)

using namespace test::bitset;

namespace {

// Empty, the lowest and highest digit, both alternations, a mixed word, and full.
constexpr auto integers = std::array{0ULL, 1ULL, 1ULL << 63U, 0x5555'5555'5555'5555ULL, 0xAAAA'AAAA'AAAA'AAAAULL, 0x0123'4567'89AB'CDEFULL, ~0ULL};

} // namespace

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstructionYieldsAnEmptyBitset, T, test::spec::bitset::every_width)
{
        constructor<T>()();
}

// A run-time width takes the integer after the count, at every count up to the sweep's limit.
BOOST_AUTO_TEST_CASE_TEMPLATE(TheIntegerConstructorSetsTheLowPositions, T, test::spec::bitset::every_width)
{
        for (auto const val : integers) {
                if constexpr (test::dynamic<T>) {
                        on1::any_value<T>([&](auto num_bits) {
                                constructor<T>()(num_bits, static_cast<unsigned long>(val));
                        });
                } else {
                        constructor<T>()(val);
                }
        }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitStringOfEveryCardinalityAndSingletonConstructsIt, T, test::spec::bitset::boundary_widths)
{
        on1::all_cardinality_sets<T>(string_constructor<T>());
        on1::all_singleton_sets<T>(string_constructor<T>());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitStringOfRandomBitsetsConstructsThem, T, test::spec::bitset::random_widths)
{
        test::spec::random::all_bitsets<T>(string_constructor<T>());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
