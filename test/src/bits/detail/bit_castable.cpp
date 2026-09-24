//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/bit_castable.hpp> // bit_bytes, bit_castable, bit_layout_holds, block_range_source, byte_count, bytes_bits, container_source, integer_source
#include <boost/test/unit_test.hpp>          // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                             // array
#include <bitset>                            // bitset
#include <cstddef>                           // byte, size_t
#include <cstdint>                           // uint8_t, uint16_t, uint32_t, uint64_t
#include <string>                            // string
#include <vector>                            // vector

BOOST_AUTO_TEST_SUITE(BitCastable)

namespace detail = xstd::bits::detail;

// An unsigned integer is its own layout: bit n of the value is 2^n by the language, so nothing is probed.
BOOST_AUTO_TEST_CASE(AnUnsignedIntegerIsItsOwnLayout)
{
        static_assert(detail::integer_source<unsigned char, 8UZ>);
        static_assert(detail::integer_source<std::uint64_t, 64UZ>);
        static_assert(detail::integer_source<std::uint64_t, 1UZ>);

        // A width the integer cannot hold is not a narrower conversion, it is none.
        static_assert(not detail::integer_source<std::uint32_t, 33UZ>);
        static_assert(not detail::integer_source<unsigned char, 9UZ>);

        // Signed is not a field of bits under this rule; nor is a type with no bits to offer.
        static_assert(not detail::integer_source<int, 8UZ>);
        static_assert(not detail::integer_source<bool, 1UZ>);
}

// The other family proves what the first states, asserted as a value so a rung where it failed reports one assertion.
BOOST_AUTO_TEST_CASE(TheLayoutIsProvedOnThisStandardLibrary)
{
        static_assert(detail::bit_layout_holds<std::bitset<200UZ>, 200UZ>());
        static_assert(detail::bit_layout_holds<std::bitset<64UZ>, 64UZ>());

        static_assert(detail::container_source<std::bitset<1UZ>, 1UZ>);
        static_assert(detail::container_source<std::bitset<32UZ>, 32UZ>); // the MSVC STL's narrow word type
        static_assert(detail::container_source<std::bitset<33UZ>, 33UZ>); // and its wide one
        static_assert(detail::container_source<std::bitset<200UZ>, 200UZ>);

        // Five fixed positions cost the same at any width, so a large one is no hazard up to 2^20.
        static_assert(detail::container_source<std::bitset<1UZ << 16UZ>, 1UZ << 16UZ>);
}

// Four layouts that are wrong, one per way, each clearing every structural bound so only the probe refuses it.
namespace wrong {

// Its default is not all clear, so the first scan refuses it.
struct dirty_default
{
        std::uint64_t w = 1ULL;

        constexpr auto set(std::size_t n) noexcept
                -> void
        {
                w |= 1ULL << n;
        }

        [[nodiscard]] static constexpr auto count() noexcept
                -> std::size_t
        {
                return 1UZ;
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 64UZ;
        }
};

// It lights the position it was asked for and one more, so the count refuses it.
struct miscounting
{
        std::uint64_t w = 0ULL;

        constexpr auto set(std::size_t n) noexcept
                -> void
        {
                w |= 1ULL << n;
        }

        [[nodiscard]] static constexpr auto count() noexcept
                -> std::size_t
        {
                return 2UZ;
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 64UZ;
        }
};

// It numbers its positions from the other end, which is the reordering the byte check refuses.
struct reversed
{
        std::uint64_t w = 0ULL;

        constexpr auto set(std::size_t n) noexcept
                -> void
        {
                w |= 1ULL << (63UZ - n);
        }

        [[nodiscard]] static constexpr auto count() noexcept
                -> std::size_t
        {
                return 1UZ;
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 64UZ;
        }
};

// It is a whole spare word wider than its positions, which is what the size window is for.
struct spare_word
{
        std::uint64_t w = 0ULL;
        std::uint64_t unused = 0ULL;

        constexpr auto set(std::size_t n) noexcept
                -> void
        {
                w |= 1ULL << n;
        }

        [[nodiscard]] static constexpr auto count() noexcept
                -> std::size_t
        {
                return 1UZ;
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 64UZ;
        }
};

// Its set() is well-formed but not a constant expression, so the probe cannot run at all rather than refusing.
struct non_constant_set
{
        std::uint64_t w = 0ULL;

        auto set(std::size_t n) noexcept -> void // NOLINT(readability-make-member-function-const)
        {
                w |= 1ULL << n;
        }

        [[nodiscard]] static constexpr auto count() noexcept
                -> std::size_t
        {
                return 1UZ;
        }

        [[nodiscard]] static constexpr auto size() noexcept
                -> std::size_t
        {
                return 64UZ;
        }
};

} // namespace wrong

BOOST_AUTO_TEST_CASE(TheProbeRefusesALayoutThatIsWrong)
{
        // static_assert and never a run-time call: the probe emits no code and has no coverage slots.
        static_assert(not detail::bit_layout_holds<wrong::dirty_default, 64UZ>());
        static_assert(not detail::bit_layout_holds<wrong::miscounting, 64UZ>());
        static_assert(not detail::bit_layout_holds<wrong::reversed, 64UZ>());

        // A set() that is no constant expression is refused before the probe runs, which is the point of the guard.
        static_assert(detail::probeable_bits<wrong::non_constant_set>);
        static_assert(not detail::probe_is_constant<wrong::non_constant_set>);
        static_assert(not detail::container_source<wrong::non_constant_set, 64UZ>);
        static_assert(not detail::bit_castable<wrong::non_constant_set, 64UZ>);

        // And the concept refuses all four, the last of them before the probe ever runs.
        static_assert(not detail::bit_castable<wrong::dirty_default, 64UZ>);
        static_assert(not detail::bit_castable<wrong::miscounting, 64UZ>);
        static_assert(not detail::bit_castable<wrong::reversed, 64UZ>);
        static_assert(not detail::bit_castable<wrong::spare_word, 64UZ>);

        // A right one, built the same way, so the four above are refused for their defect and not their shape.
        struct right
        {
                std::uint64_t w = 0ULL;

                constexpr auto set(std::size_t n) noexcept
                        -> void
                {
                        w |= 1ULL << n;
                }

                [[nodiscard]] static constexpr auto count() noexcept
                        -> std::size_t
                {
                        return 1UZ;
                }

                [[nodiscard]] static constexpr auto size() noexcept
                        -> std::size_t
                {
                        return 64UZ;
                }
        };

        static_assert(detail::bit_layout_holds<right, 64UZ>());
        static_assert(detail::bit_castable<right, 64UZ>);
}

// Neither family: a heap container is not its own bits, and a width the object cannot hold is not a conversion.
BOOST_AUTO_TEST_CASE(WhatIsRefusedAndWhy)
{
        static_assert(not detail::bit_castable<std::vector<bool>, 64UZ>);
        static_assert(not detail::bit_castable<std::string, 64UZ>);
        static_assert(not detail::bit_castable<std::bitset<64UZ>, 65UZ>);
        static_assert(not detail::bit_castable<std::uint32_t, 64UZ>);
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

// The byte view is the whole object and nothing besides, refusing a reordered word and a big-endian target both.
BOOST_AUTO_TEST_CASE(OnePositionLightsOneBitOfOneByte)
{
        constexpr auto N = 200UZ;
        for (auto i = 0UZ; i < N; ++i) {
                auto bs = std::bitset<N>();
                bs.set(i);
                auto const bytes = detail::bit_bytes<N>(bs);
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        BOOST_CHECK(bytes[j] == (j == i / 8UZ ? static_cast<std::byte>(1U << (i % 8UZ)) : std::byte{}));
                }
        }
}

// A contiguous sequence of blocks is the same stated family over more than one word, and nothing is probed here.
BOOST_AUTO_TEST_CASE(ASequenceOfBlocksStatesItsLayoutToo)
{
        static_assert(detail::block_range_source<std::array<std::uint64_t, 4>, 256UZ>);
        static_assert(detail::block_range_source<std::array<std::uint32_t, 8>, 256UZ>);
        static_assert(detail::block_range_source<std::array<std::uint8_t, 32>, 256UZ>);

        // AT LEAST N, the rule the scalar spelling already follows: wider is admitted, narrower is no conversion.
        static_assert(detail::block_range_source<std::array<std::uint64_t, 5>, 256UZ>);
        static_assert(not detail::block_range_source<std::array<std::uint64_t, 3>, 256UZ>);

        // A width that is not a whole number of blocks still only needs enough blocks to cover it.
        static_assert(detail::block_range_source<std::array<std::uint64_t, 2>, 65UZ>);
        static_assert(not detail::block_range_source<std::array<std::uint64_t, 1>, 65UZ>);

        // A vector has no bits until one is put in it, so B().size() is zero and it states nothing.
        static_assert(not detail::block_range_source<std::vector<std::uint64_t>, 64UZ>);

        // Signed blocks are not this family: contiguous_block_range asks for an unsigned value type.
        static_assert(not detail::block_range_source<std::array<int, 4>, 64UZ>);

        // And a scalar is not a range, which is why the family keeps two spellings rather than one.
        static_assert(not detail::block_range_source<std::uint64_t, 64UZ>);
        static_assert(detail::bit_castable<std::uint64_t, 64UZ>);
}

// The bytes a block sequence spells are the bytes of its values, so b[j] >> k is the same on either byte order.
BOOST_AUTO_TEST_CASE(BlocksAndBytesAreEachOthersInverse)
{
        using Blocks = std::array<std::uint64_t, 2>;
        constexpr auto N = 128UZ;

        static_assert([] -> bool {
                auto const blocks = Blocks{0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL};
                return detail::bytes_bits<Blocks, N>(detail::bit_bytes<N>(blocks)) == blocks;
        }());

        // Byte j of the field is byte j % 8 of block j / 8, said as a shift on the value.
        static_assert([] -> bool {
                auto const blocks = Blocks{0x0000'0000'0000'FF01ULL, 0x0000'0000'0000'0002ULL};
                auto const bytes = detail::bit_bytes<N>(blocks);
                return bytes[0] == std::byte{0x01} and bytes[1] == std::byte{0xFF} and bytes[2] == std::byte{0x00} and bytes[8] == std::byte{0x02};
        }());

        // Two block widths over the same positions spell the same bytes, which is the whole claim.
        static_assert([] -> bool {
                auto const wide = std::array<std::uint64_t, 1>{0x0123'4567'89AB'CDEFULL};
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
                using Wide = std::array<std::uint64_t, 2>;
                using Narrow = std::array<std::uint8_t, 16>;

                constexpr auto wide = Wide{0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL};
                constexpr auto folded = detail::bit_bytes<N>(wide);
                auto const copied = detail::bit_bytes<N>(wide);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<Wide, N>(folded);
                auto const back_copied = detail::bytes_bits<Wide, N>(folded);
                BOOST_CHECK(back_copied == back_folded);
                BOOST_CHECK(back_copied == wide);

                constexpr auto narrow_folded = detail::bytes_bits<Narrow, N>(folded);
                auto const narrow_copied = detail::bytes_bits<Narrow, N>(folded);
                BOOST_CHECK(narrow_copied == narrow_folded);
        }

        // A bare unsigned integer, at a width narrower than the value, so the copy takes fewer bytes than sizeof.
        {
                constexpr auto N = 32UZ;
                constexpr auto value = 0xDEAD'BEEFULL;
                constexpr auto folded = detail::bit_bytes<N>(value);
                auto const copied = detail::bit_bytes<N>(value);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<unsigned long long, N>(folded);
                auto const back_copied = detail::bytes_bits<unsigned long long, N>(folded);
                BOOST_CHECK_EQUAL(back_copied, back_folded);
                BOOST_CHECK_EQUAL(back_copied, value);
        }

        // A foreign field of bits at a width that is not a whole number of bytes, so the last byte is a partial one.
        {
                constexpr auto N = 100UZ;
                using Field = std::bitset<N>;

                constexpr auto field = Field(0x0F1E'2D3C'4B5A'6978ULL);
                constexpr auto folded = detail::bit_bytes<N>(field);
                auto const copied = detail::bit_bytes<N>(field);
                BOOST_CHECK(copied == folded);

                constexpr auto back_folded = detail::bytes_bits<Field, N>(folded);
                auto const back_copied = detail::bytes_bits<Field, N>(folded);
                BOOST_CHECK(back_copied == back_folded);
                BOOST_CHECK(back_copied == field);
        }
}

BOOST_AUTO_TEST_SUITE_END()
