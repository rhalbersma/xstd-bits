//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit/bit_convert.hpp>                // bit_convert
#include <xstd/bits/bit_array.hpp>                      // bit_array
#include <xstd/bits/bit_fixed_set.hpp>                  // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>               // bit_flag_mapping
#include <xstd/bits/bit_flag_set.hpp>                   // bit_flag_set
#include <xstd/bits/bit_set.hpp>                        // basic_bit_set, bit_set
#include <xstd/bits/bit_type_traits/bit_align.hpp>      // bit_align
#include <xstd/bits/bit_type_traits/bit_fast.hpp>       // bit_fast
#include <xstd/bits/bit_type_traits/bit_least.hpp>      // bit_least
#include <xstd/bits/bit_type_traits/bit_underlying.hpp> // bit_underlying, underlying_block_t
#include <xstd/bits/from_blocks.hpp>                    // from_blocks
#include <boost/test/unit_test.hpp>                     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                                        // array
#include <bit>                                          // bit_cast
#include <bitset>                                       // bitset
#include <cstddef>                                      // size_t
#include <cstdint>                                      // int16_t, int64_t, int8_t, uint16_t, uint32_t, uint64_t, uint8_t
#include <functional>                                   // greater
#include <type_traits>                                  // is_same_v
#include <utility>                                      // to_underlying

// Named rather than unnamed, as a concept reads these operators and the mapping without calling them.
namespace wire {

// Flags as an ABI stores them, in a 32-bit block with a flag at either end of it.
enum class flag : std::uint32_t
{
        none  = 0x0000'0000,
        read  = 0x0000'0001,
        write = 0x0000'0002,
        exec  = 0x0000'0004,
        high  = 0x8000'0000,
};

// A signed underlying type, whose sign bit is a flag like the others.
enum class signed_flag : std::int16_t
{
        low  = 0x0001,
        sign = -0x8000,
};

// The operators [bitmask.types] asks for, over the two flag enumerations, so each is a bit mask a flag set takes.
template<class Flag>
concept wire_mask = std::is_same_v<Flag, flag> or std::is_same_v<Flag, signed_flag>;

template<wire_mask Flag>
[[nodiscard]] constexpr auto operator~(Flag a) noexcept
        -> Flag
{
        return static_cast<Flag>(~std::to_underlying(a));
}

template<wire_mask Flag>
[[nodiscard]] constexpr auto operator&(Flag a, Flag b) noexcept
        -> Flag
{
        return static_cast<Flag>(std::to_underlying(a) & std::to_underlying(b));
}

template<wire_mask Flag>
[[nodiscard]] constexpr auto operator|(Flag a, Flag b) noexcept
        -> Flag
{
        return static_cast<Flag>(std::to_underlying(a) | std::to_underlying(b));
}

template<wire_mask Flag>
[[nodiscard]] constexpr auto operator^(Flag a, Flag b) noexcept
        -> Flag
{
        return static_cast<Flag>(std::to_underlying(a) ^ std::to_underlying(b));
}

template<wire_mask Flag>
constexpr auto operator&=(Flag& a, Flag b) noexcept
        -> Flag&
{
        return a = a & b;
}

template<wire_mask Flag>
constexpr auto operator|=(Flag& a, Flag b) noexcept
        -> Flag&
{
        return a = a | b;
}

template<wire_mask Flag>
constexpr auto operator^=(Flag& a, Flag b) noexcept
        -> Flag&
{
        return a = a ^ b;
}

// Plain char and bool as underlying types: the one has an unsigned counterpart, the other none.
enum class glyph : char
{
        a = 'a',
};

enum class yes_no : bool
{
        no,
        yes,
};

// Declared and never defined: a mapping keying a set by yes_no, for a concept to read.
struct yes_no_mapping
{
        [[nodiscard]] static auto to_index(yes_no key) noexcept -> std::size_t;
        [[nodiscard]] static auto from_index(std::size_t index) noexcept -> yes_no;
};

} // namespace wire

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitUnderlying)

namespace {

template<class W>
concept underlying_rebinds = requires { typename xstd::bit_underlying<W>; };

template<class Enum>
concept has_underlying_block = requires { typename xstd::underlying_block_t<Enum>; };

// Values of the ABI's block: none, single flags at either end, and every flag at once.
constexpr auto wire_values = std::array{wire::flag::none, wire::flag::read, wire::flag::exec, wire::flag::high, std::bit_cast<wire::flag>(0x8000'0007U)};

} // namespace

// An enumeration's underlying type or an integer type made unsigned is a block; bool, a character type or a class none.
BOOST_AUTO_TEST_CASE(TheUnderlyingBlockIsTheUnderlyingTypeMadeUnsigned)
{
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::flag>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::signed_flag>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::glyph>, unsigned char>);
        static_assert(std::is_same_v<xstd::underlying_block_t<int>, unsigned> and std::is_same_v<xstd::underlying_block_t<std::int8_t>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<std::uint32_t>, std::uint32_t> and std::is_same_v<xstd::underlying_block_t<std::int64_t>, std::uint64_t>);
        static_assert(not has_underlying_block<wire::yes_no> and not has_underlying_block<std::bitset<8>>);
        static_assert(not has_underlying_block<bool> and not has_underlying_block<char> and not has_underlying_block<char8_t> and not has_underlying_block<wchar_t>);
        BOOST_CHECK(true);
}

// A flag set in its enumeration's own block: the block an ABI stores, whatever block or width it was spelled in.
BOOST_AUTO_TEST_CASE(TheUnderlyingSetIsInTheEnumerationsOwnBlock)
{
        using flags = xstd::bit_flag_set<wire::flag>;
        using fast  = xstd::bit_fast<flags>;
        static_assert(std::is_same_v<flags, xstd::basic_bit_fixed_set<wire::flag, std::uint32_t, 32, xstd::bit_flag_mapping<wire::flag>, std::greater<wire::flag>>>); // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::is_same_v<xstd::bit_underlying<fast>, flags>);
        static_assert(std::is_same_v<xstd::bit_underlying<flags>, flags>);
        static_assert(std::is_same_v<xstd::bit_align<xstd::bit_underlying<fast>>, flags>);
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::bit_flag_set<wire::flag, 9>>, xstd::basic_bit_fixed_set<wire::flag, std::uint32_t, 9, xstd::bit_flag_mapping<wire::flag, 9>, std::greater<wire::flag>>>); // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::bit_flag_set<wire::signed_flag>>, xstd::bit_flag_set<wire::signed_flag>>);
        static_assert(sizeof(xstd::bit_underlying<fast>) == sizeof(std::uint32_t) and sizeof(xstd::bit_underlying<xstd::bit_flag_set<wire::flag, 9>>) == sizeof(std::uint32_t));

        // The block is the mask's representation bit for bit, so either way across is a copy.
        for (auto const mask : wire_values) {
                auto const x = xstd::bit_underlying<fast>(mask);
                BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint32_t>(x), std::to_underlying(mask));
                BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(x), std::to_underlying(mask));
                BOOST_CHECK(xstd::bit_underlying<fast>(xstd::from_blocks, std::to_underlying(mask)) == mask);
                BOOST_CHECK(wire::flag(xstd::bit_underlying<fast>(xstd::from_blocks, std::to_underlying(mask))) == mask);
        }
}

// An integer mask's set in the unsigned counterpart: a narrow set of int widened back to the block an int field stores.
BOOST_AUTO_TEST_CASE(TheUnderlyingSetOfAnIntegerMaskIsInItsUnsignedCounterpart)
{
        using narrow = xstd::bit_flag_set<int, 5>;
        static_assert(std::is_same_v<xstd::bit_underlying<narrow>, xstd::basic_bit_fixed_set<int, unsigned, 5, xstd::bit_flag_mapping<int, 5>, std::greater<int>>>); // NOLINT(modernize-use-transparent-functors): the comparator the alias names
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::bit_flag_set<std::int8_t>>, xstd::bit_flag_set<std::int8_t>>);
        static_assert(sizeof(narrow) == 1UZ and sizeof(xstd::bit_underlying<narrow>) == sizeof(int));
        auto const x = xstd::bit_underlying<narrow>(0b10110);
        BOOST_CHECK_EQUAL(std::bit_cast<unsigned>(x), 0b10110U);
        BOOST_CHECK(int(x) == 0b10110 and x == narrow(0b10110));
}

// Only a set keyed by an enumeration or an integer with an unsigned counterpart has an underlying block.
BOOST_AUTO_TEST_CASE(OnlyASetOfEnumerationOrIntegerKeysHasAnUnderlyingBlock)
{
        static_assert(underlying_rebinds<xstd::bit_flag_set<wire::flag>> and underlying_rebinds<xstd::bit_flag_set<wire::signed_flag, 3>>);
        static_assert(not underlying_rebinds<xstd::bit_array<9>> and std::is_same_v<xstd::bit_underlying<xstd::bit_set>, xstd::bit_set>);
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::basic_bit_set<wire::flag, std::uint8_t, xstd::bit_flag_mapping<wire::flag>, std::greater<wire::flag>>>, xstd::basic_bit_set<wire::flag, std::uint32_t, xstd::bit_flag_mapping<wire::flag>, std::greater<wire::flag>>>); // NOLINT(modernize-use-transparent-functors): the comparator the type names
        static_assert(std::is_same_v<xstd::bit_underlying<xstd::bit_least<xstd::bit_fixed_set<9>>>, xstd::bit_fixed_set<9>>);
        static_assert(not underlying_rebinds<xstd::bit_flag_set<std::bitset<16>>>);
        static_assert(not underlying_rebinds<xstd::basic_bit_fixed_set<wire::yes_no, std::uint8_t, 2, wire::yes_no_mapping>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
