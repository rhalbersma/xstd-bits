//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/inplace_vector.hpp>                  // IWYU pragma: keep; TEST_HAS_INPLACE_VECTOR
#include <test/minimal_blocks.hpp>                  // minimal_blocks
#include <xstd/bits/bit/bit_convert.hpp>            // bit_convert
#include <xstd/bits/bit_array.hpp>                  // bit_array
#include <xstd/bits/bit_blocks.hpp>                 // bit_align, bit_block, bit_block_range, bit_blocks, bit_blocks_capacity_v, bit_blocks_extent_v, bit_fast, bit_least, bit_underlying, fast_block_t, least_block_t, owned_bit_blocks, resizable_bit_blocks, underlying_block_t
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>           // bit_flag_mapping
#include <xstd/bits/bit_flag_set.hpp>               // bit_flag_set
#include <xstd/bits/bit_set.hpp>                    // bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/detail/bit_block_container.hpp> // bit_block_container
#include <xstd/bits/from_blocks.hpp>                // from_blocks
#include <boost/container/small_vector.hpp>         // small_vector
#include <boost/container/static_vector.hpp>        // static_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                    // array
#include <bit>                                      // bit_cast
#include <bitset>                                   // bitset
#include <cstddef>                                  // size_t
#include <cstdint>                                  // int16_t, uint16_t, uint32_t, uint64_t, uint8_t, uint_fast16_t, uint_fast32_t, uint_fast64_t, uint_fast8_t
#include <deque>                                    // deque
#include <functional>                               // greater
#include <list>                                     // list
#include <span>                                     // dynamic_extent, span
#include <type_traits>                              // is_same_v
#include <utility>                                  // to_underlying
#include <vector>                                   // vector

#ifdef TEST_HAS_INPLACE_VECTOR

#include <inplace_vector> // inplace_vector

#endif

// Named rather than unnamed, as a concept reads these operators and the mapping without calling them.
namespace wire {

// Flags as an ABI stores them, in a 32-bit word with a flag at either end of it.
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

BOOST_AUTO_TEST_SUITE(BitBlocks)

namespace {

template<class W>
concept holds_blocks = requires { typename xstd::bits::detail::bit_block_container<W>; };

template<class W>
concept names_a_view = requires { typename xstd::bit_set_view<W>; };

template<class W, std::size_t N>
concept holds_extent = requires { typename xstd::bits::detail::bit_block_container<W, N>; };

template<class W>
concept least_rebinds = requires { typename xstd::bit_least<W>; };

template<class W>
concept fast_rebinds = requires { typename xstd::bit_fast<W>; };

template<class W>
concept align_rebinds = requires { typename xstd::bit_align<W>; };

template<class W>
concept underlying_rebinds = requires { typename xstd::bit_underlying<W>; };

template<class Enum>
concept has_underlying_block = requires { typename xstd::underlying_block_t<Enum>; };

// Values of the ABI's word: none, single flags at either end, and every flag at once.
constexpr auto wire_values = std::array{wire::flag::none, wire::flag::read, wire::flag::exec, wire::flag::high, std::bit_cast<wire::flag>(0x8000'0007U)};

// Built-in arrays of blocks, named once so the storage under test is spelled where the check can be told why.
using four_words        = std::uint64_t[4];       // NOLINT(modernize-avoid-c-arrays): the storage under test
using three_const_words = std::uint16_t const[3]; // NOLINT(modernize-avoid-c-arrays): the storage under test

} // namespace

// A block is bit storage, and so is a sized contiguous range of blocks: every storage the containers hold.
BOOST_AUTO_TEST_CASE(BlocksAndContiguousRangesOfBlocksAreBitStorage)
{
        static_assert(xstd::bit_blocks<std::uint8_t> and xstd::bit_blocks<std::uint64_t> and xstd::bit_blocks<std::uint64_t const>);
        static_assert(xstd::bit_blocks<std::array<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks<std::vector<std::size_t>>);
        static_assert(xstd::bit_blocks<std::span<std::uint32_t>> and xstd::bit_blocks<std::span<std::uint32_t const, 2>>);
        static_assert(xstd::bit_blocks<four_words> and xstd::bit_blocks<three_const_words>);
#ifdef TEST_HAS_INPLACE_VECTOR
        static_assert(xstd::bit_blocks<std::inplace_vector<std::uint16_t, 3>>);
#endif
        BOOST_CHECK(true);
}

// Bit blocks are one block or a range of them, and the two halves are named apart.
BOOST_AUTO_TEST_CASE(BitBlocksAreABlockOrARangeOfThem)
{
        static_assert(xstd::bit_block<std::uint64_t> and xstd::bit_block<std::uint64_t const> and xstd::bit_block<std::uint8_t volatile>);
        static_assert(not xstd::bit_block<int> and not xstd::bit_block<bool> and not xstd::bit_block<std::array<std::uint64_t, 1>>);
        static_assert(xstd::bit_block_range<std::array<std::uint16_t, 3>> and xstd::bit_block_range<std::span<std::uint32_t const>>);
        static_assert(xstd::bit_block_range<std::vector<std::size_t>> and xstd::bit_block_range<four_words>);
        static_assert(not xstd::bit_block_range<std::uint64_t> and not xstd::bit_block_range<std::deque<std::uint32_t>>);
        BOOST_CHECK(true);
}

// A packed container has bit storage and is not bit storage, and neither is anything not laid out as blocks.
BOOST_AUTO_TEST_CASE(EverythingElseIsNot)
{
        static_assert(not xstd::bit_blocks<int> and not xstd::bit_blocks<bool> and not xstd::bit_blocks<double>);
        static_assert(not xstd::bit_blocks<std::vector<int>> and not xstd::bit_blocks<std::vector<bool>>);
        static_assert(not xstd::bit_blocks<std::deque<std::uint32_t>> and not xstd::bit_blocks<std::list<std::uint32_t>>);
        static_assert(not xstd::bit_blocks<std::bitset<64>>);
        static_assert(not xstd::bit_blocks<xstd::bit_set> and not xstd::bit_blocks<xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

// The storage and the views are named by bit storage and nothing else.
BOOST_AUTO_TEST_CASE(TheStorageAndTheViewsAreNamedByBitStorage)
{
        static_assert(holds_blocks<std::vector<std::uint32_t>> and holds_blocks<std::array<std::uint64_t, 1>>);
        static_assert(names_a_view<std::uint64_t const> and names_a_view<std::span<std::uint32_t>>);
        static_assert(not holds_blocks<std::bitset<64>> and not holds_blocks<xstd::bit_set>);
        static_assert(not names_a_view<std::vector<bool>> and not names_a_view<int>);
        BOOST_CHECK(true);
}

// The width storage names by its type, which the views default to: fixed blocks have one, the rest do not.
BOOST_AUTO_TEST_CASE(TheExtentIsTheWidthTheTypeNames)
{
        static_assert(xstd::bit_blocks_extent_v<std::uint8_t> == 8 and xstd::bit_blocks_extent_v<std::uint64_t const> == 64);
        static_assert(xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3>> == 48 and xstd::bit_blocks_extent_v<std::array<std::uint16_t, 3> const> == 48);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t, 2>> == 64 and xstd::bit_blocks_extent_v<std::span<std::uint32_t const, 2>> == 64);
        static_assert(xstd::bit_blocks_extent_v<four_words> == 256 and xstd::bit_blocks_extent_v<three_const_words> == 48);
        static_assert(xstd::bit_blocks_extent_v<four_words> == xstd::bit_blocks_extent_v<std::array<std::uint64_t, 4>>);
        static_assert(xstd::bit_blocks_extent_v<std::span<std::uint32_t>> == std::dynamic_extent);
        static_assert(xstd::bit_blocks_extent_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bit_set_view<std::span<std::uint32_t>>, xstd::bit_set_view<std::span<std::uint32_t>, std::dynamic_extent>>);
        BOOST_CHECK(true);
}

// The narrowest fixed-width block holding N bits, the widest taking every N above it in several blocks.
BOOST_AUTO_TEST_CASE(TheLeastBlockIsTheNarrowestHoldingTheWidth)
{
        static_assert(std::is_same_v<xstd::least_block_t<0>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::least_block_t<8>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::least_block_t<9>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::least_block_t<16>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::least_block_t<17>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::least_block_t<32>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::least_block_t<33>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<64>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<65>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<1000>, std::uint64_t>);
        BOOST_CHECK(true);
}

// The fastest block of at least N bits is <cstdint>'s, whose width the platform chooses, so only its name is fixed.
BOOST_AUTO_TEST_CASE(TheFastBlockIsTheFastestOfAtLeastTheWidth)
{
        static_assert(std::is_same_v<xstd::fast_block_t<0>, std::uint_fast8_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<8>, std::uint_fast8_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<9>, std::uint_fast16_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<16>, std::uint_fast16_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<17>, std::uint_fast32_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<32>, std::uint_fast32_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<33>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<64>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<65>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<1000>, std::uint_fast64_t>);
        BOOST_CHECK(true);
}

// The transformations rewrite a width in the type, so a run-time width, a view or a bare block has none to give.
BOOST_AUTO_TEST_CASE(OnlyAFixedWidthOwnerIsTransformed)
{
        static_assert(least_rebinds<xstd::bit_array<9>> and fast_rebinds<xstd::bit_array<9>> and align_rebinds<xstd::bit_array<9>>);
        static_assert(not least_rebinds<xstd::bit_set> and not fast_rebinds<xstd::bit_set> and not align_rebinds<xstd::bit_set>);
        static_assert(not least_rebinds<xstd::bit_set_view<std::span<std::uint32_t>>> and not align_rebinds<xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not least_rebinds<std::bitset<9>> and not fast_rebinds<std::uint64_t> and not align_rebinds<std::array<std::uint8_t, 2>>);
        BOOST_CHECK(true);
}

// An enumeration's underlying type made unsigned is a block; bool has no unsigned counterpart, and a non-enum none.
BOOST_AUTO_TEST_CASE(TheUnderlyingBlockIsTheUnderlyingTypeMadeUnsigned)
{
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::flag>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::signed_flag>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::underlying_block_t<wire::glyph>, unsigned char>);
        static_assert(not has_underlying_block<wire::yes_no> and not has_underlying_block<std::uint32_t> and not has_underlying_block<std::bitset<8>>);
        BOOST_CHECK(true);
}

// A flag set in its enumeration's own word: the block an ABI stores, whatever block or width it was spelled in.
BOOST_AUTO_TEST_CASE(TheUnderlyingSetIsInTheEnumerationsOwnWord)
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

        // The word is the mask's representation bit for bit, so either way across is a copy.
        for (auto const mask : wire_values) {
                auto const x = xstd::bit_underlying<fast>(mask);
                BOOST_CHECK_EQUAL(xstd::bit_convert<std::uint32_t>(x), std::to_underlying(mask));
                BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(x), std::to_underlying(mask));
                BOOST_CHECK(xstd::bit_underlying<fast>(xstd::from_blocks, std::to_underlying(mask)) == mask);
                BOOST_CHECK(wire::flag(xstd::bit_underlying<fast>(xstd::from_blocks, std::to_underlying(mask))) == mask);
        }
}

// Only a fixed-width set keyed by an enumeration with an unsigned counterpart has an underlying word to take.
BOOST_AUTO_TEST_CASE(OnlyASetOfEnumerationKeysHasAnUnderlyingWord)
{
        static_assert(underlying_rebinds<xstd::bit_flag_set<wire::flag>> and underlying_rebinds<xstd::bit_flag_set<wire::signed_flag, 3>>);
        static_assert(not underlying_rebinds<xstd::bit_array<9>> and not underlying_rebinds<xstd::bit_fixed_set<9>> and not underlying_rebinds<xstd::bit_set>);
        static_assert(not underlying_rebinds<xstd::bit_flag_set<std::bitset<16>>>);
        static_assert(not underlying_rebinds<xstd::basic_bit_fixed_set<wire::yes_no, std::uint8_t, 2, wire::yes_no_mapping>>);
        BOOST_CHECK(true);
}

// An owner takes what compares by its blocks and stays read-only through const; a span is neither, so views take it.
BOOST_AUTO_TEST_CASE(OwnedStorageIsAValueThatConstKeepsReadOnly)
{
        static_assert(xstd::owned_bit_blocks<std::uint64_t> and xstd::owned_bit_blocks<std::array<std::uint16_t, 3>>);
        static_assert(xstd::owned_bit_blocks<std::vector<std::size_t>>);
        static_assert(not xstd::owned_bit_blocks<std::span<std::uint32_t>> and not xstd::owned_bit_blocks<std::span<std::uint32_t, 2>>);
        static_assert(not xstd::owned_bit_blocks<std::uint64_t const> and not xstd::owned_bit_blocks<std::array<std::uint16_t, 3> const>);
        static_assert(xstd::bit_blocks<std::span<std::uint32_t>> and xstd::bit_blocks<std::uint64_t const>);

        // A built-in array is bit storage and no value: it neither assigns nor compares, so no owner holds one.
        static_assert(not xstd::owned_bit_blocks<four_words> and not xstd::owned_bit_blocks<three_const_words>);
        static_assert(not holds_blocks<four_words> and not holds_extent<four_words, 256>);
        BOOST_CHECK(true);
}

// A run-time width grows its blocks, so an owner at one takes only storage that resizes; a fixed width takes any.
BOOST_AUTO_TEST_CASE(ARunTimeWidthOwnsOnlyStorageThatResizes)
{
        static_assert(xstd::resizable_bit_blocks<std::vector<std::size_t>>);
        static_assert(not xstd::resizable_bit_blocks<std::array<std::uint64_t, 2>> and not xstd::resizable_bit_blocks<std::uint64_t>);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::resizable_bit_blocks<std::inplace_vector<std::uint16_t, 3>>);

#endif
        static_assert(xstd::resizable_bit_blocks<test::minimal_blocks<std::uint32_t>>);
        static_assert(holds_extent<std::vector<std::size_t>, std::dynamic_extent>);
        static_assert(holds_extent<test::minimal_blocks<std::uint32_t>, std::dynamic_extent>);
        static_assert(holds_extent<std::array<std::uint64_t, 2>, 100> and not holds_extent<std::array<std::uint64_t, 2>, std::dynamic_extent>);
        static_assert(not holds_extent<std::uint64_t, std::dynamic_extent>);
        BOOST_CHECK(true);
}

// An owner's N is its bound: a fixed width, a constant capacity its blocks hold in whole, or none at all.
BOOST_AUTO_TEST_CASE(AnOwnersExtentIsItsWidthOrItsCapacity)
{
        static_assert(xstd::bit_blocks_capacity_v<std::uint64_t> == 64 and xstd::bit_blocks_capacity_v<std::array<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_blocks_capacity_v<std::vector<std::size_t>> == std::dynamic_extent);
        static_assert(xstd::bit_blocks_capacity_v<test::minimal_blocks<std::uint32_t>> == std::dynamic_extent);
#ifdef TEST_HAS_INPLACE_VECTOR

        static_assert(xstd::bit_blocks_capacity_v<std::inplace_vector<std::uint16_t, 3>> == 48);
        static_assert(xstd::bit_blocks_extent_v<std::inplace_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(std::is_same_v<xstd::bits::detail::bit_block_container<std::inplace_vector<std::uint16_t, 3>>, xstd::bits::detail::bit_block_container<std::inplace_vector<std::uint16_t, 3>, 48>>);

        // Any capacity the blocks hold in whole: stopping inside the last block, but never short of it.
        static_assert(holds_extent<std::inplace_vector<std::uint16_t, 3>, 33> and holds_extent<std::inplace_vector<std::uint16_t, 3>, 47>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, 32> and not holds_extent<std::inplace_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<std::inplace_vector<std::uint16_t, 3>, std::dynamic_extent>);

#endif
        static_assert(not holds_extent<std::vector<std::size_t>, 64>);
        BOOST_CHECK(true);
}

// A static capacity() callable only at run time still bounds the type, through the static_capacity beside it.
BOOST_AUTO_TEST_CASE(AStaticVectorsCapacityIsItsStaticCapacity)
{
        static_assert(xstd::resizable_bit_blocks<boost::container::static_vector<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks_capacity_v<boost::container::static_vector<std::uint16_t, 3>> == 48);
        static_assert(std::is_same_v<xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>>, xstd::bits::detail::bit_block_container<boost::container::static_vector<std::uint16_t, 3>, 48>>);
        static_assert(holds_extent<boost::container::static_vector<std::uint16_t, 3>, 33> and not holds_extent<boost::container::static_vector<std::uint16_t, 3>, 49>);
        static_assert(not holds_extent<boost::container::static_vector<std::uint16_t, 3>, std::dynamic_extent>);

        // A small_vector's static_capacity is only what it holds before it allocates, so it bounds nothing.
        static_assert(xstd::resizable_bit_blocks<boost::container::small_vector<std::uint16_t, 3>>);
        static_assert(xstd::bit_blocks_capacity_v<boost::container::small_vector<std::uint16_t, 3>> == std::dynamic_extent);
        static_assert(holds_extent<boost::container::small_vector<std::uint16_t, 3>, std::dynamic_extent>);
        static_assert(not holds_extent<boost::container::small_vector<std::uint16_t, 3>, 48>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
