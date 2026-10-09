//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/bit_layout.hpp> // bit_bytes, bit_layout, byte_count, bytes_bits, container_source, fixed_bit_blocks, fixed_blocks_source
#include <xstd/bits/detail/bit_width.hpp>  // bit_width_v
#include <boost/test/unit_test.hpp>        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                           // array, to_array
#include <bitset>                          // bitset
#include <cstddef>                         // byte, size_t
#include <cstdint>                         // uint8_t, uint16_t, uint32_t, uint64_t
#include <ranges>                          // iota
#include <span>                            // dynamic_extent, span
#include <string>                          // string
#include <vector>                          // vector

BOOST_AUTO_TEST_SUITE(BitLayout)

namespace detail = xstd::bits::detail;

namespace {

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_blocks = std::uint64_t[4]; // NOLINT(modernize-avoid-c-arrays): the storage under test
using four_ints   = int[4];           // NOLINT(modernize-avoid-c-arrays): the storage under test

// Two blocks with no member a field of bits would be asked for, read only through their object bytes.
struct raw
{
        std::uint64_t lo;
        std::uint64_t hi;

        [[nodiscard]] friend auto operator==(raw const&, raw const&) noexcept -> bool = default;
};

} // namespace

// An unsigned integer is its own layout: bit n of the value is 2^n by the language, whatever the byte order.
BOOST_AUTO_TEST_CASE(AnUnsignedIntegerIsItsOwnLayout)
{
        static_assert(detail::fixed_blocks_source<unsigned char, 8UZ>);
        static_assert(detail::fixed_blocks_source<std::uint64_t, 64UZ>);
        static_assert(detail::fixed_blocks_source<std::uint64_t, 1UZ>);

        // A width the integer cannot hold is not a narrower conversion, it is none.
        static_assert(not detail::fixed_blocks_source<std::uint32_t, 33UZ>);
        static_assert(not detail::fixed_blocks_source<unsigned char, 9UZ>);

        // Signed is not a field of bits under this rule; nor is a type with no bits to offer.
        static_assert(not detail::fixed_blocks_source<int, 8UZ>);
        static_assert(not detail::fixed_blocks_source<bool, 1UZ>);
}

// The other family is read as its bytes wherever its object has room for the width, as std::bit_cast trusts it.
BOOST_AUTO_TEST_CASE(AFieldOfBitsIsReadWhereItsObjectHasRoom)
{
        static_assert(detail::container_source<std::bitset<1UZ>, 1UZ>);
        static_assert(detail::container_source<std::bitset<32UZ>, 32UZ>); // the MSVC STL's narrow block type
        static_assert(detail::container_source<std::bitset<33UZ>, 33UZ>); // and its wide one
        static_assert(detail::container_source<std::bitset<200UZ>, 200UZ>);
        static_assert(detail::container_source<std::bitset<1UZ << 16UZ>, 1UZ << 16UZ>);

        // Nothing is asked of its members, so plain blocks with room for the width are a field of bits too.
        static_assert(detail::container_source<raw, 100UZ>);
        static_assert(detail::bit_layout<raw, 128UZ>);
}

// Neither family: a heap container is not its own bits, and a width the object cannot hold is not a conversion.
BOOST_AUTO_TEST_CASE(WhatIsRefusedAndWhy)
{
        static_assert(not detail::bit_layout<std::vector<bool>, 64UZ>);
        static_assert(not detail::bit_layout<std::string, 64UZ>);
        static_assert(not detail::bit_layout<std::bitset<64UZ>, 65UZ>);
        static_assert(not detail::bit_layout<std::uint32_t, 64UZ>);
        static_assert(not detail::bit_layout<raw, 129UZ>);
}

// A width is in a foreign type only as std::bitset spells it; any other foreign type takes its width from a target.
BOOST_AUTO_TEST_CASE(AForeignWidthIsAStdBitsetsOnly)
{
        static_assert(detail::bit_width_v<std::bitset<0UZ>> == 0UZ);
        static_assert(detail::bit_width_v<std::bitset<100UZ>> == 100UZ);
        static_assert(detail::bit_width_v<std::bitset<100UZ> const> == 100UZ);
        static_assert(detail::bit_width_v<raw> == std::dynamic_extent);
}

BOOST_AUTO_TEST_CASE(TheTwoDirectionsAreEachOthersInverse)
{
        auto const round_trips = []<class B, std::size_t N>(B const& b) -> bool {
                return detail::bytes_bits<B, N>(detail::bit_bytes<N>(b)) == b;
        };

        auto wide = std::bitset<200UZ>();
        for (auto i = 0UZ; i < 200UZ; i += 3UZ) {
                wide.set(i);
        }
        BOOST_CHECK((round_trips.template operator()<std::bitset<200UZ>, 200UZ>(wide)));
        BOOST_CHECK((round_trips.template operator()<std::bitset<200UZ>, 200UZ>(std::bitset<200UZ>())));
        BOOST_CHECK((round_trips.template operator()<std::bitset<200UZ>, 200UZ>(std::bitset<200UZ>().flip())));

        // The integer family, at a width that fills the type and one that does not.
        BOOST_CHECK((round_trips.template operator()<std::uint64_t, 64UZ>(0xDEADBEEFCAFEF00DULL)));
        BOOST_CHECK((round_trips.template operator()<std::uint64_t, 64UZ>(0ULL)));
        BOOST_CHECK((round_trips.template operator()<std::uint8_t, 8UZ>(static_cast<std::uint8_t>(0xA5U))));

        // Zero width, which has no byte to exchange and round trips all the same.
        BOOST_CHECK((round_trips.template operator()<std::bitset<0UZ>, 0UZ>(std::bitset<0UZ>())));
        static_assert(detail::byte_count<0UZ> == 0UZ);
}

// The byte view is the whole object and nothing besides, refusing a reordered block and a big-endian target both.
BOOST_AUTO_TEST_CASE(OnePositionLightsOneBitOfOneByte)
{
        constexpr auto N = 200UZ;
        for (auto const i : std::views::iota(0UZ, N)) {
                auto bs = std::bitset<N>();
                bs.set(i);
                auto const bytes = detail::bit_bytes<N>(bs);
                for (auto const j : std::views::iota(0UZ, bytes.size())) {
                        BOOST_CHECK(bytes[j] == (j == i / 8UZ ? static_cast<std::byte>(1U << (i % 8UZ)) : std::byte{}));
                }
        }
}

// Bit blocks whose type names their width: a block, an array of blocks, and a span of a static extent.
BOOST_AUTO_TEST_CASE(FixedBitBlocksNameTheirWidthByType)
{
        static_assert(detail::fixed_bit_blocks<std::uint64_t>);
        static_assert(detail::fixed_bit_blocks<std::array<std::uint32_t, 3>>);
        static_assert(detail::fixed_bit_blocks<std::span<std::uint16_t, 2>>);
        static_assert(not detail::fixed_bit_blocks<std::span<std::uint16_t>>);
        static_assert(not detail::fixed_bit_blocks<std::vector<std::uint64_t>>);
}

// A contiguous sequence of blocks is the same stated family over more than one block.
BOOST_AUTO_TEST_CASE(ASequenceOfBlocksStatesItsLayoutToo)
{
        static_assert(detail::fixed_blocks_source<std::array<std::uint64_t, 4>, 256UZ>);
        static_assert(detail::fixed_blocks_source<std::array<std::uint32_t, 8>, 256UZ>);
        static_assert(detail::fixed_blocks_source<std::array<std::uint8_t, 32>, 256UZ>);

        // AT LEAST N, the rule the scalar spelling already follows: wider is admitted, narrower is no conversion.
        static_assert(detail::fixed_blocks_source<std::array<std::uint64_t, 5>, 256UZ>);
        static_assert(not detail::fixed_blocks_source<std::array<std::uint64_t, 3>, 256UZ>);

        // A width that is not a whole number of blocks still only needs enough blocks to cover it.
        static_assert(detail::fixed_blocks_source<std::array<std::uint64_t, 2>, 65UZ>);
        static_assert(not detail::fixed_blocks_source<std::array<std::uint64_t, 1>, 65UZ>);

        // A vector names no width by its type, so it states nothing.
        static_assert(not detail::fixed_blocks_source<std::vector<std::uint64_t>, 64UZ>);

        // Signed blocks are not this family: owned_bit_blocks asks for an unsigned value type.
        static_assert(not detail::fixed_blocks_source<std::array<int, 4>, 64UZ>);

        // A built-in array is read as its std::array; signed blocks state no layout, so theirs is their object bytes.
        static_assert(detail::fixed_blocks_source<four_blocks, 256UZ> and not detail::fixed_blocks_source<four_blocks, 257UZ>);
        static_assert(detail::bit_layout<four_blocks, 256UZ> and not detail::bit_layout<four_blocks, 257UZ>);
        static_assert(not detail::fixed_blocks_source<four_ints, 64UZ> and detail::container_source<four_ints, 64UZ>);
        static_assert([] -> bool {
                four_blocks const blocks = {0x0123'4567'89AB'CDEFULL, 0x0ULL, 0x1ULL, 0x8000'0000'0000'0000ULL};
                auto const same          = std::array<std::uint64_t, 4>{0x0123'4567'89AB'CDEFULL, 0x0ULL, 0x1ULL, 0x8000'0000'0000'0000ULL};
                return detail::bit_bytes<256UZ>(blocks) == detail::bit_bytes<256UZ>(same);
        }());

        // At run time too, where the blocks cross as one copy rather than by shifts.
        static constexpr four_blocks blocks = {0x0123'4567'89AB'CDEFULL, 0x0ULL, 0x1ULL, 0x8000'0000'0000'0000ULL};
        BOOST_CHECK(detail::bit_bytes<256UZ>(blocks) == detail::bit_bytes<256UZ>(std::to_array(blocks)));

        // And a scalar is the length-one case of the same family.
        static_assert(detail::fixed_blocks_source<std::uint64_t, 64UZ>);
        static_assert(detail::bit_layout<std::uint64_t, 64UZ>);
}

// The bytes a block sequence spells are the bytes of its values, so b[j] >> k is the same on either byte order.
BOOST_AUTO_TEST_CASE(BlocksAndBytesAreEachOthersInverse)
{
        using Blocks     = std::array<std::uint64_t, 2>;
        constexpr auto N = 128UZ;

        static_assert([] -> bool {
                auto const blocks = Blocks{0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL};
                return detail::bytes_bits<Blocks, N>(detail::bit_bytes<N>(blocks)) == blocks;
        }());

        // Byte j of the field is byte j % 8 of block j / 8, said as a shift on the value.
        static_assert([] -> bool {
                auto const blocks = Blocks{0x0000'0000'0000'FF01ULL, 0x0000'0000'0000'0002ULL};
                auto const bytes  = detail::bit_bytes<N>(blocks);
                return bytes[0] == std::byte{0x01} and bytes[1] == std::byte{0xFF} and bytes[2] == std::byte{0x00} and bytes[8] == std::byte{0x02};
        }());

        // Two block widths over the same positions spell the same bytes, which is the whole claim.
        static_assert([] -> bool {
                auto const wide   = std::array<std::uint64_t, 1>{0x0123'4567'89AB'CDEFULL};
                auto const narrow = std::array<std::uint8_t, 8>{0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
                return detail::bit_bytes<64UZ>(wide) == detail::bit_bytes<64UZ>(narrow);
        }());

        // Blocks above the width come back clear, which is what makes the round trip an identity at a wider sequence.
        static_assert([] -> bool {
                auto const out = detail::bytes_bits<std::array<std::uint64_t, 4>, 64UZ>(
                        detail::bit_bytes<64UZ>(std::array<std::uint64_t, 4>{7ULL, 1ULL, 1ULL, 1ULL})
                );
                return out[0] == 7ULL and out[1] == 0ULL and out[2] == 0ULL and out[3] == 0ULL;
        }());
}

// The copy and the shifts must agree: if consteval is a language rule, so the two genuinely run different code.
BOOST_AUTO_TEST_CASE(TheCopyAndTheShiftsAgree)
{
        // A sequence of blocks, at two block widths, so the bytes-per-block arithmetic is exercised either side.
        {
                constexpr auto N = 128UZ;
                using Wide       = std::array<std::uint64_t, 2>;
                using Narrow     = std::array<std::uint8_t, 16>;

                constexpr auto wide   = Wide{0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL};
                constexpr auto folded = detail::bit_bytes<N>(wide);
                auto const copied     = detail::bit_bytes<N>(wide);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<Wide, N>(folded);
                auto const back_copied     = detail::bytes_bits<Wide, N>(folded);
                BOOST_CHECK(back_copied == back_folded);
                BOOST_CHECK(back_copied == wide);

                constexpr auto narrow_folded = detail::bytes_bits<Narrow, N>(folded);
                auto const narrow_copied     = detail::bytes_bits<Narrow, N>(folded);
                BOOST_CHECK(narrow_copied == narrow_folded);
        }

        // A bare unsigned integer, at a width narrower than the value, so the copy takes fewer bytes than sizeof.
        {
                constexpr auto N      = 32UZ;
                constexpr auto value  = 0xDEAD'BEEFULL;
                constexpr auto folded = detail::bit_bytes<N>(value);
                auto const copied     = detail::bit_bytes<N>(value);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<unsigned long long, N>(folded);
                auto const back_copied     = detail::bytes_bits<unsigned long long, N>(folded);
                BOOST_CHECK_EQUAL(back_copied, back_folded);
                BOOST_CHECK_EQUAL(back_copied, value);
        }

        // A foreign field of bits at a width that is not a whole number of bytes, so the last byte is a partial one.
        {
                constexpr auto N = 100UZ;
                using Field      = std::bitset<N>;

                constexpr auto field  = Field(0x0F1E'2D3C'4B5A'6978ULL);
                constexpr auto folded = detail::bit_bytes<N>(field);
                auto const copied     = detail::bit_bytes<N>(field);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<Field, N>(folded);
                auto const back_copied     = detail::bytes_bits<Field, N>(folded);
                BOOST_CHECK(back_copied == back_folded);
                BOOST_CHECK(back_copied == field);
        }

        // Plain blocks at 100 bits cross as their first 13 bytes, and the 3 bytes past those come back zero.
        {
                constexpr auto N    = 100UZ;
                constexpr auto ones = ~0ULL;
                constexpr auto kept = raw{.lo = ones, .hi = 0xFF'FFFF'FFFFULL};

                constexpr auto blocks = raw{.lo = ones, .hi = ones};
                constexpr auto folded = detail::bit_bytes<N>(blocks);
                auto const copied     = detail::bit_bytes<N>(blocks);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<raw, N>(folded);
                auto const back_copied     = detail::bytes_bits<raw, N>(folded);
                static_assert(back_folded == kept);
                BOOST_CHECK(back_copied == kept);
        }
}

BOOST_AUTO_TEST_SUITE_END()
