//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>                          // perm
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp> // bit_mask_mapping
#include <xstd/bits/bit_flag_mapping.hpp>              // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>               // bit_key_mapping, bit_range_mapping
#include <boost/test/unit_test.hpp>                    // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bitset>                                      // bitset
#include <cstddef>                                     // byte, size_t
#include <cstdint>                                     // uint8_t
#include <future>                                      // launch

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitMaskMapping)

// The flag mapping reads every value as a mask of its keys, its block holding them at their positions.
BOOST_AUTO_TEST_CASE(TheFlagMappingReadsAValueAsAMask)
{
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::byte>, std::byte>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::byte, 2UZ>, std::byte>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::launch>, std::launch>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::bitset<16>>, std::bitset<16>>);
        static_assert(xstd::bit_flag_mapping<std::byte>::to_block(std::byte{0x02}) == 0x02U);
        static_assert(xstd::bit_flag_mapping<std::byte>::from_block(0x03U) == std::byte{0x03});

        BOOST_CHECK(true);
}

// The identity, a range and a listed enumeration rank their values, and the flags of a byte are not a uint8_t's.
BOOST_AUTO_TEST_CASE(NoOtherMappingReadsAValueAsAMask)
{
        static_assert(not xstd::bit_mask_mapping<xstd::bit_key_mapping<std::size_t>, std::size_t>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_range_mapping<std::uint8_t, 0, 8UZ>, std::uint8_t>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_key_mapping<test::set::perm>, test::set::perm>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::byte>, std::uint8_t>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
