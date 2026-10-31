//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                // block_types
#include <test/for_each_type.hpp>              // for_each_type
#include <xstd/bits/bit/has_single_bit.hpp>    // has_single_bit
#include <xstd/bits/bit_set/bit_fixed_set.hpp> // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_set.hpp>       // bit_set
#include <xstd/bits/bit_vector.hpp>            // bit_vector
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t
#include <ranges>                              // iota

BOOST_AUTO_TEST_SUITE(HasSingleBit)

// One position anywhere, in the first block, a middle one or the last, is a single bit; none is not.
BOOST_AUTO_TEST_CASE(OnePositionAnywhereIsASingleBit)
{
        test::for_each_type<test::block_types>([]<class Block> -> void {
                using set = xstd::basic_bit_fixed_set<std::size_t, Block, 257>;
                BOOST_CHECK(not xstd::has_single_bit(set()));
                for (auto const i : std::views::iota(0UZ, 257UZ)) {
                        BOOST_CHECK(xstd::has_single_bit(set{i}));
                }
        });
}

// Two positions are never one, in the same block or in two, the later one in any block after the first.
BOOST_AUTO_TEST_CASE(TwoPositionsAreNotASingleBit)
{
        test::for_each_type<test::block_types>([]<class Block> -> void {
                using set = xstd::basic_bit_fixed_set<std::size_t, Block, 130>;
                for (auto const i : std::views::iota(0UZ, 130UZ)) {
                        for (auto const j : std::views::iota(i + 1UZ, 130UZ)) {
                                BOOST_CHECK(not xstd::has_single_bit(set{i, j}));
                        }
                }
        });
}

// Over a run-time width and over a sequence, it answers as a count of one does.
BOOST_AUTO_TEST_CASE(AgreesWithACountOfOne)
{
        for (auto const n : {0UZ, 1UZ, 63UZ, 64UZ, 65UZ, 200UZ}) {
                auto keys = xstd::bit_set();
                auto bits = xstd::bit_vector(n);
                auto ones = 0UZ;
                for (auto const i : std::views::iota(0UZ, n)) {
                        if (i % 7UZ == 3UZ) {
                                keys.insert(i);
                                bits[i] = true;
                                ++ones;
                        }
                        BOOST_CHECK_EQUAL(xstd::has_single_bit(keys), keys.size() == 1UZ);
                        BOOST_CHECK_EQUAL(xstd::has_single_bit(bits), ones == 1UZ);
                }
        }
}

// Usable in a constant expression for a fixed width, beside xstd-ints' overload over one integer it brings.
BOOST_AUTO_TEST_CASE(IsConstantAndKeepsTheIntegerOverload)
{
        static_assert(xstd::has_single_bit(xstd::bit_fixed_set<192>{100}));
        static_assert(not xstd::has_single_bit(xstd::bit_fixed_set<192>{100, 150}));
        static_assert(xstd::has_single_bit(std::uint64_t{1} << 40U));
        static_assert(not xstd::has_single_bit(std::uint64_t{3}));
}

BOOST_AUTO_TEST_SUITE_END()
