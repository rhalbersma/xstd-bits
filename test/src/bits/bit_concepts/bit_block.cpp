//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_concepts/bit_block.hpp> // bit_block
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                // array
#include <cstdint>                              // uint64_t, uint8_t

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitBlock)

// A block is one unsigned integer, cv-qualified or not; a signed one, bool and an array of blocks are none.
BOOST_AUTO_TEST_CASE(ABlockIsOneUnsignedInteger)
{
        static_assert(xstd::bit_block<std::uint64_t> and xstd::bit_block<std::uint64_t const> and xstd::bit_block<std::uint8_t volatile>);
        static_assert(not xstd::bit_block<int> and not xstd::bit_block<bool> and not xstd::bit_block<std::array<std::uint64_t, 1>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
