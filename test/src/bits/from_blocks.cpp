//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>                                  // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit/bit_convert.hpp>                     // bit_convert
#include <xstd/bits/bit_array.hpp>                           // basic_bit_array, bit_array
#include <xstd/bits/bit_concepts/bit_constructible_from.hpp> // bit_constructible_from
#include <xstd/bits/bit_fixed_set.hpp>                       // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_vector.hpp>                          // bit_vector
#include <xstd/bits/from_blocks.hpp>                         // from_blocks, from_blocks_t
#include <boost/test/unit_test.hpp>                          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                             // array
#include <bitset>                                            // bitset
#include <concepts>                                          // same_as
#include <cstddef>                                           // size_t
#include <cstdint>                                           // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                                            // iota
#include <type_traits>                                       // is_constructible_v, is_default_constructible_v

BOOST_AUTO_TEST_SUITE(FromBlocks)

namespace {

template<class T>
concept deduces_from_blocks_of = requires (T const& value) { xstd::basic_bit_array(xstd::from_blocks, value); };

// Declared only, for the concept below to call in an unevaluated operand.
template<class T>
[[maybe_unused]] auto from_braces(T)
        -> void;

template<class T>
concept braces_convert = requires { from_braces<T>({}); };

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks = std::uint64_t[4];      // NOLINT(modernize-avoid-c-arrays): the storage under test
using three_bytes = std::uint8_t const[3]; // NOLINT(modernize-avoid-c-arrays): the storage under test
using two_halves  = std::uint32_t[2];      // NOLINT(modernize-avoid-c-arrays): the storage under test
using one_block   = std::uint64_t[1];      // NOLINT(modernize-avoid-c-arrays): the storage under test
using two_ints    = int[2];                // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// The tag is std::from_range's twin: an explicit default constructor, so {} cannot stand in for it.
BOOST_AUTO_TEST_CASE(TheTagIsExplicitlyDefaultConstructible)
{
        static_assert(std::is_default_constructible_v<xstd::from_blocks_t>);
        static_assert(not braces_convert<xstd::from_blocks_t>);
        static_assert(std::same_as<decltype(xstd::from_blocks), xstd::from_blocks_t const>);
        BOOST_CHECK(true);
}

// An integer's digits are the width, and its value's bits are the positions, at every reading.
BOOST_AUTO_TEST_CASE(AnIntegerDeducesItsOwnWidth)
{
        constexpr auto block = std::uint16_t{0b1000'0000'0000'0101};

        constexpr auto a = xstd::basic_bit_array(xstd::from_blocks, block);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint16_t, 16> const>);
        static_assert(a == xstd::basic_bit_array<std::uint16_t, 16>(xstd::from_blocks, block));
        static_assert(a[0] and not a[1] and a[2] and a[15]);

        constexpr auto s = xstd::basic_bit_fixed_set(xstd::from_blocks, block);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 16> const>);
        static_assert(s == xstd::basic_bit_fixed_set<std::size_t, std::uint16_t, 16>(xstd::from_blocks, block));
        static_assert(s.size() == 3UZ and s.contains(15UZ));
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(a) == block);
}

// An array of blocks is its blocks' width, block i holding positions [i * digits, (i + 1) * digits).
BOOST_AUTO_TEST_CASE(AnArrayOfBlocksDeducesTheirWidth)
{
        constexpr auto blocks = std::array<std::uint8_t, 3>{0x01, 0x00, 0x80};

        constexpr auto a = xstd::basic_bit_array(xstd::from_blocks, blocks);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint8_t, 24> const>);
        static_assert(a[0] and a[23] and a.count() == 2UZ);

        constexpr auto s = xstd::basic_bit_fixed_set(xstd::from_blocks, blocks);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 24> const>);
        static_assert(s.contains(0UZ) and s.contains(23UZ) and s.size() == 2UZ);
        BOOST_CHECK((a == xstd::basic_bit_array<std::uint8_t, 24>(xstd::from_blocks, blocks)));
}

// A built-in array of blocks reads as the std::array of the same blocks, and deduces the same width.
BOOST_AUTO_TEST_CASE(ABuiltInArrayReadsAsTheStdArrayOfItsBlocks)
{
        static constexpr four_blocks blocks = {0x8000'0000'0000'0001ULL, 0x0ULL, 0xF0ULL, 0x8000'0000'0000'0000ULL};
        constexpr auto same                 = std::array<std::uint64_t, 4>{0x8000'0000'0000'0001ULL, 0x0ULL, 0xF0ULL, 0x8000'0000'0000'0000ULL};

        constexpr auto a = xstd::basic_bit_array(xstd::from_blocks, blocks);
        static_assert(std::same_as<decltype(a), decltype(xstd::basic_bit_array(xstd::from_blocks, same)) const>);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint64_t, 256> const>);
        static_assert(a == xstd::basic_bit_array<std::uint64_t, 256>(xstd::from_blocks, same));

        constexpr auto s = xstd::basic_bit_fixed_set(xstd::from_blocks, blocks);
        static_assert(std::same_as<decltype(s), decltype(xstd::basic_bit_fixed_set(xstd::from_blocks, same)) const>);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 256> const>);
        static_assert(s == xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 256>(xstd::from_blocks, same));

        // Position by position, and at a narrower width, which reads the blocks without deducing one.
        auto const narrow     = xstd::bit_array<200>(xstd::from_blocks, blocks);
        auto const narrow_std = xstd::bit_array<200>(xstd::from_blocks, same);
        auto const keys       = xstd::bit_fixed_set<256>(xstd::from_blocks, blocks);
        for (auto const i : std::views::iota(0UZ, 256UZ)) {
                auto const expected = ((same[i / 64UZ] >> (i % 64UZ)) & 1U) != 0U;
                BOOST_CHECK(a[i] == expected);
                BOOST_CHECK(keys.contains(i) == expected);
                if (i < 200UZ) {
                        BOOST_CHECK(narrow[i] == narrow_std[i]);
                }
        }

        // A const array deduces too, and no built-in array is storage an owner takes as it is; signed blocks are none.
        static_assert(deduces_from_blocks_of<three_bytes>);
        static_assert(std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_blocks_t, two_halves const&>);
        static_assert(not xstd::bit_constructible_from<xstd::bit_fixed_set<64>, one_block>);
        static_assert(not deduces_from_blocks_of<two_ints>);
}

// Signed integers are no field of bits, and a run-time width names none; an empty array names width zero.
BOOST_AUTO_TEST_CASE(OnlyAnUnsignedIntegerOrItsArrayDeduces)
{
        static_assert(deduces_from_blocks_of<std::uint64_t>);
        static_assert(deduces_from_blocks_of<std::array<std::uint32_t, 2>>);
        static_assert(std::same_as<decltype(xstd::basic_bit_array(xstd::from_blocks, std::array<std::uint32_t, 0>())), xstd::basic_bit_array<std::uint32_t, 0>>);
        static_assert(std::same_as<decltype(xstd::basic_bit_fixed_set(xstd::from_blocks, std::array<std::uint32_t, 0>())), xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 0>>);
        static_assert(not deduces_from_blocks_of<int>);
        static_assert(not deduces_from_blocks_of<std::array<int, 2>>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::from_blocks_t, std::uint64_t>);
#ifdef TEST_HAS_UINT128

        // Wider than unsigned long long, and still a field of bits the tag reads.
        static_assert(deduces_from_blocks_of<xstd::uint128>);
        constexpr auto wide = xstd::basic_bit_array(xstd::from_blocks, xstd::uint128{1} << 100U);
        static_assert(std::same_as<decltype(wide), xstd::basic_bit_array<xstd::uint128, 128> const>);
        static_assert(wide[100] and wide.count() == 1UZ);

#endif
        BOOST_CHECK(true);
}

// Only what is bit storage is read through the tag: a std::bitset has bit storage and is not it, so it is converted.
BOOST_AUTO_TEST_CASE(OnlyWhatIsBitStorageIsReadThroughTheTag)
{
        static_assert(std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_blocks_t, std::uint64_t>);
        static_assert(std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_blocks_t, std::array<std::uint32_t, 2>>);
        static_assert(not std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_blocks_t, std::bitset<64>>);
        static_assert(not std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_blocks_t, xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
