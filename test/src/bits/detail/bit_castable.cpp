//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/detail/bit_castable.hpp>  // bit_bytes, bit_castable, bit_layout_holds, block_range_source, byte_count, bytes_bits, container_source, integer_source
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                              // array
#include <bitset>                             // bitset
#include <cstddef>                            // byte, size_t
#include <cstdint>                            // uint8_t, uint16_t, uint32_t, uint64_t
#include <string>                             // string
#include <vector>                             // vector

BOOST_AUTO_TEST_SUITE(BitCastable)

namespace bits = xstd::detail::bits;

// An unsigned integer is its own layout: bit n of the value is 2^n by the language, so this family is admitted
// on arithmetic alone, with nothing probed and nothing assumed about any implementation.
BOOST_AUTO_TEST_CASE(AnUnsignedIntegerIsItsOwnLayout)
{
        static_assert(bits::integer_source<unsigned char,       8UZ>);
        static_assert(bits::integer_source<std::uint64_t,      64UZ>);
        static_assert(bits::integer_source<std::uint64_t,       1UZ>);

        // A width the integer cannot hold is not a narrower conversion, it is none.
        static_assert(not bits::integer_source<std::uint32_t,  33UZ>);
        static_assert(not bits::integer_source<unsigned char,   9UZ>);

        // Signed is not a field of bits under this rule; nor is a type with no bits to offer.
        static_assert(not bits::integer_source<int,             8UZ>);
        static_assert(not bits::integer_source<bool,            1UZ>);
}

// The other family proves what the first states. Every standard library this ladder builds against lays a
// std::bitset out as ascending words, least significant bit first, so byte n / 8 holds position n at bit n % 8 --
// asserted here as a value rather than a static_assert, so a rung where it were ever false reports one failing
// assertion instead of failing to compile the whole target.
BOOST_AUTO_TEST_CASE(TheLayoutIsProvedOnThisStandardLibrary)
{
        static_assert(bits::bit_layout_holds<std::bitset<200UZ>, 200UZ>());
        static_assert(bits::bit_layout_holds<std::bitset< 64UZ>,  64UZ>());

        static_assert(bits::container_source<std::bitset<  1UZ>,   1UZ>);
        static_assert(bits::container_source<std::bitset< 32UZ>,  32UZ>);   // the MSVC STL's narrow word type
        static_assert(bits::container_source<std::bitset< 33UZ>,  33UZ>);   // and its wide one
        static_assert(bits::container_source<std::bitset<200UZ>, 200UZ>);

        // Five fixed positions cost the same whatever the width, so a large one is not a compile-time hazard --
        // up to 2^20, past which clang's default -fconstexpr-steps wants raising.
        static_assert(bits::container_source<std::bitset<1UZ << 16UZ>, 1UZ << 16UZ>);
}

// Four layouts that are WRONG, one per way of being wrong, so that what the probe refuses is tested rather than
// assumed. Each is trivially copyable and answers set/count/size, so each clears every structural bound and is
// refused by the probe alone -- which is the only evidence that the probe is doing the work the concept credits
// it with. They are also what covers its four refusals: a conforming implementation takes none of them.
namespace wrong {

// Its default is not all clear, so the first scan refuses it.
struct dirty_default
{
        std::uint64_t w = 1ULL;
        constexpr auto set(std::size_t n) noexcept -> void { w |= 1ULL << n; }
        [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 1UZ; }
        [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
};

// It lights the position it was asked for and one more, so the count refuses it.
struct miscounting
{
        std::uint64_t w = 0ULL;
        constexpr auto set(std::size_t n) noexcept -> void { w |= 1ULL << n; }
        [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 2UZ; }
        [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
};

// It numbers its positions from the other end, which is the reordering the byte check refuses.
struct reversed
{
        std::uint64_t w = 0ULL;
        constexpr auto set(std::size_t n) noexcept -> void { w |= 1ULL << (63UZ - n); }
        [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 1UZ; }
        [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
};

// It is a whole spare word wider than its positions, which is what the size window is for.
struct spare_word
{
        std::uint64_t w = 0ULL;
        std::uint64_t unused = 0ULL;
        constexpr auto set(std::size_t n) noexcept -> void { w |= 1ULL << n; }
        [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 1UZ; }
        [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
};


// Its set() is well-formed but NOT usable in a constant expression, which is a different failure from every one
// above: those are layouts the probe RUNS and refuses, this is one the probe cannot run at all. A block type whose
// operator|= is not constexpr produces exactly this -- absl::uint128 is one, and a bitset over it reaches here
// through its own reading. Without a guard the probe is a hard error in the middle of a constraint; with one it is
// an ordinary unsatisfied concept, and the conversion simply does not exist for that type.
struct non_constant_set
{
        std::uint64_t w = 0ULL;
        auto set(std::size_t n) noexcept -> void { w |= 1ULL << n; }        // NOLINT(readability-make-member-function-const)
        [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 1UZ; }
        [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
};

}       // namespace wrong

BOOST_AUTO_TEST_CASE(TheProbeRefusesALayoutThatIsWrong)
{
        // static_assert and never a run-time call: the probe is constexpr-ONLY by design, so it emits no code and
        // has no coverage slots. Calling it once at run time emits one instantiation per type probed, and no single
        // one of those can take every arm -- a correct layout takes none of the refusals, and an incorrect one
        // returns at the first and never reaches the rest. Measured, before it reached CI.
        static_assert(not bits::bit_layout_holds<wrong::dirty_default, 64UZ>());
        static_assert(not bits::bit_layout_holds<wrong::miscounting,   64UZ>());
        static_assert(not bits::bit_layout_holds<wrong::reversed,      64UZ>());

        // A set() that is not a CONSTANT EXPRESSION is refused without the probe running, which is the whole point:
        // it is everything probeable_bits asks for, so the probe would otherwise be reached and hard-error.
        static_assert(    bits::probeable_bits<wrong::non_constant_set>);
        static_assert(not bits::probe_is_constant<wrong::non_constant_set>);
        static_assert(not bits::container_source<wrong::non_constant_set, 64UZ>);
        static_assert(not bits::bit_castable<wrong::non_constant_set, 64UZ>);

        // And the concept refuses all four, the last of them before the probe ever runs.
        static_assert(not bits::bit_castable<wrong::dirty_default, 64UZ>);
        static_assert(not bits::bit_castable<wrong::miscounting,   64UZ>);
        static_assert(not bits::bit_castable<wrong::reversed,      64UZ>);
        static_assert(not bits::bit_castable<wrong::spare_word,    64UZ>);

        // A right one, built the same way, so the four above are refused for their defect and not their shape.
        struct right
        {
                std::uint64_t w = 0ULL;
                constexpr auto set(std::size_t n) noexcept -> void { w |= 1ULL << n; }
                [[nodiscard]] static constexpr auto count() noexcept -> std::size_t { return 1UZ; }
                [[nodiscard]] static constexpr auto size() noexcept -> std::size_t { return 64UZ; }
        };
        static_assert(bits::bit_layout_holds<right, 64UZ>());
        static_assert(bits::bit_castable<right, 64UZ>);
}

// Neither family, and for a different reason each: a heap container is not its own bits, and a width the object
// cannot hold is not a conversion.
BOOST_AUTO_TEST_CASE(WhatIsRefusedAndWhy)
{
        static_assert(not bits::bit_castable<std::vector<bool>, 64UZ>);
        static_assert(not bits::bit_castable<std::string,       64UZ>);
        static_assert(not bits::bit_castable<std::bitset<64UZ>, 65UZ>);
        static_assert(not bits::bit_castable<std::uint32_t,     64UZ>);
}

BOOST_AUTO_TEST_CASE(TheTwoDirectionsAreEachOthersInverse)
{
        auto const round_trips = []<class B, std::size_t N>(B const& b) -> bool {
                return bits::bytes_bits<B, N>(bits::bit_bytes<N>(b)) == b;
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
        BOOST_CHECK((round_trips.template operator()<std::uint8_t,   8UZ>(static_cast<std::uint8_t>(0xA5U))));

        // Zero width, which has no byte to exchange and round trips all the same.
        BOOST_CHECK((round_trips.template operator()<std::bitset<0UZ>, 0UZ>(std::bitset<0UZ>())));
        static_assert(bits::byte_count<0UZ> == 0UZ);
}

// The byte view is the whole of the object and nothing besides: one position lights one bit of one byte and
// leaves every other byte clear. That second half is what refuses a reordered word and a big-endian target both.
BOOST_AUTO_TEST_CASE(OnePositionLightsOneBitOfOneByte)
{
        constexpr auto N = 200UZ;
        for (auto i = 0UZ; i < N; ++i) {
                auto bs = std::bitset<N>();
                bs.set(i);
                auto const bytes = bits::bit_bytes<N>(bs);
                for (auto j = 0UZ; j < bytes.size(); ++j) {
                        BOOST_CHECK(bytes[j] == (j == i / 8UZ ? static_cast<std::byte>(1U << (i % 8UZ)) : std::byte{}));
                }
        }
}


// A CONTIGUOUS SEQUENCE OF BLOCKS is the same stated family said over more than one word: block j holds the
// positions [j*digits, (j+1)*digits), and a scalar is the sequence of length one. Nothing is probed here either.
BOOST_AUTO_TEST_CASE(ASequenceOfBlocksStatesItsLayoutToo)
{
        static_assert(bits::block_range_source<std::array<std::uint64_t, 4>, 256UZ>);
        static_assert(bits::block_range_source<std::array<std::uint32_t, 8>, 256UZ>);
        static_assert(bits::block_range_source<std::array<std::uint8_t, 32>, 256UZ>);

        // AT LEAST N, the rule the scalar spelling already follows: wider is admitted, narrower is no conversion.
        static_assert(    bits::block_range_source<std::array<std::uint64_t, 5>, 256UZ>);
        static_assert(not bits::block_range_source<std::array<std::uint64_t, 3>, 256UZ>);

        // A width that is not a whole number of blocks still only needs enough blocks to cover it.
        static_assert(    bits::block_range_source<std::array<std::uint64_t, 2>, 65UZ>);
        static_assert(not bits::block_range_source<std::array<std::uint64_t, 1>, 65UZ>);

        // A vector has no bits until one is put in it, so B().size() is zero and it states nothing. That is a
        // width and not a preference: the readings promise at COMPILE time that nothing truncates, and a run-time
        // size cannot keep that promise.
        static_assert(not bits::block_range_source<std::vector<std::uint64_t>, 64UZ>);

        // Signed blocks are not this family: contiguous_block_range asks for an unsigned value type, which is the
        // same invariant Block itself rests on.
        static_assert(not bits::block_range_source<std::array<int, 4>, 64UZ>);

        // And a scalar is not a range, which is why the family keeps two spellings rather than one.
        static_assert(not bits::block_range_source<std::uint64_t, 64UZ>);
        static_assert(    bits::bit_castable<std::uint64_t, 64UZ>);
}

// The bytes a block sequence spells are the bytes of its values, which is what keeps this endian-independent:
// b[j] >> k is the same number on either byte order, where a bit_cast of the object would not be.
BOOST_AUTO_TEST_CASE(BlocksAndBytesAreEachOthersInverse)
{
        using Blocks = std::array<std::uint64_t, 2>;
        constexpr auto N = 128UZ;

        static_assert([] -> bool {
                auto const blocks = Blocks{ 0x0123'4567'89AB'CDEFULL, 0xFEDC'BA98'7654'3210ULL };
                return bits::bytes_bits<Blocks, N>(bits::bit_bytes<N>(blocks)) == blocks;
        }());

        // Byte j of the field is byte j % 8 of block j / 8, said as a shift on the value.
        static_assert([] -> bool {
                auto const blocks = Blocks{ 0x0000'0000'0000'FF01ULL, 0x0000'0000'0000'0002ULL };
                auto const bytes  = bits::bit_bytes<N>(blocks);
                return bytes[0] == std::byte{ 0x01 }
                   and bytes[1] == std::byte{ 0xFF }
                   and bytes[2] == std::byte{ 0x00 }
                   and bytes[8] == std::byte{ 0x02 };
        }());

        // Two block widths over the same positions spell the same bytes, which is the whole claim.
        static_assert([] -> bool {
                auto const wide   = std::array<std::uint64_t, 1>{ 0x0123'4567'89AB'CDEFULL };
                auto const narrow = std::array<std::uint8_t,  8>{ 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01 };
                return bits::bit_bytes<64UZ>(wide) == bits::bit_bytes<64UZ>(narrow);
        }());

        // Blocks above the width come back CLEAR, which is what makes the round trip an identity at a width the
        // sequence is wider than.
        static_assert([] -> bool {
                auto const out = bits::bytes_bits<std::array<std::uint64_t, 4>, 64UZ>(
                        bits::bit_bytes<64UZ>(std::array<std::uint64_t, 4>{ 7ULL, 1ULL, 1ULL, 1ULL })
                );
                return out[0] == 7ULL and out[1] == 0ULL and out[2] == 0ULL and out[3] == 0ULL;
        }());
}

BOOST_AUTO_TEST_SUITE_END()
