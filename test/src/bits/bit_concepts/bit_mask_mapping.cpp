//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>                                 // perm
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp>        // bit_mask_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_flag_mapping.hpp>                     // bit_flag_mapping
#include <xstd/bits/bit_key_mapping.hpp>                      // bit_key_mapping, bit_range_mapping
#include <boost/test/unit_test.hpp>                           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <bit>                                                // bit_cast
#include <bitset>                                             // bitset
#include <cstddef>                                            // size_t
#include <cstdint>                                            // int8_t, uint8_t

// Declared and never defined: a concept reads their declarations, and an unnamed namespace would warn of the unused.
namespace nonmapping {

// A sized mapping whose block goes out to a mask and never comes back, so no mask mapping.
struct to_block_only
{
        using block_type = std::uint8_t;

        static constexpr std::size_t size = 8;

        [[nodiscard]] static auto is_key(std::uint8_t key) noexcept -> bool;
        [[nodiscard]] static auto to_index(std::uint8_t key) noexcept -> std::size_t;
        [[nodiscard]] static auto from_index(std::size_t index) noexcept -> std::uint8_t;
        [[nodiscard]] static auto to_block(std::uint8_t mask) noexcept -> block_type;
};

// A sized mapping whose block is signed, which no set holds its bits in.
struct signed_block
{
        using block_type = std::int8_t;

        static constexpr std::size_t size = 8;

        [[nodiscard]] static auto is_key(std::uint8_t key) noexcept -> bool;
        [[nodiscard]] static auto to_index(std::uint8_t key) noexcept -> std::size_t;
        [[nodiscard]] static auto from_index(std::size_t index) noexcept -> std::uint8_t;
        [[nodiscard]] static auto to_block(std::uint8_t mask) noexcept -> block_type;
        [[nodiscard]] static auto from_block(block_type block) noexcept -> std::uint8_t;
};

} // namespace nonmapping

BOOST_AUTO_TEST_SUITE(BitConcepts)
BOOST_AUTO_TEST_SUITE(BitMaskMapping)

namespace {

// A bitmask enumeration, each enumerator one bit.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
};

// A key whose arithmetic is its underlying type's, in a range that starts below zero.
enum class storey : std::int8_t
{
        basement = -3,
        roof     = 4,
};

} // namespace

// The flag mapping alone reads every value as a mask of its keys, its block holding them at their positions.
BOOST_AUTO_TEST_CASE(OnlyTheFlagMappingReadsAValueAsAMask)
{
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<mode>, mode>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<mode, 2UZ>, mode>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::bitset<16>>, std::bitset<16>>);
        static_assert(xstd::bit_flag_mapping<mode>::to_block(mode::write) == 0x02U);
        static_assert(xstd::bit_flag_mapping<mode>::from_block(0x03U) == std::bit_cast<mode>(std::uint8_t{0x03}));
        static_assert(not xstd::bit_mask_mapping<xstd::bit_flag_mapping<mode>, storey>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_key_mapping<test::set::perm>, test::set::perm>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_key_mapping<std::size_t>, std::size_t>);
        static_assert(not xstd::bit_mask_mapping<xstd::bit_range_mapping<storey, storey::basement, 8UZ>, storey>);
        static_assert(not xstd::bit_mask_mapping<nonmapping::to_block_only, std::uint8_t>);
        static_assert(not xstd::bit_mask_mapping<nonmapping::signed_block, std::uint8_t>);
        static_assert(xstd::sized_bit_index_mapping<nonmapping::to_block_only, std::uint8_t>);
        static_assert(xstd::sized_bit_index_mapping<nonmapping::signed_block, std::uint8_t>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
