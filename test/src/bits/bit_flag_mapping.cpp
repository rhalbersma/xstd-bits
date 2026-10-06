//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_fixed_set.hpp>     // basic_bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>  // bit_flag_mapping
#include <xstd/bits/bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/from_blocks.hpp>       // from_blocks
#include <boost/test/unit_test.hpp>        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                       // ranges::equal
#include <array>                           // array
#include <bit>                             // bit_cast
#include <bitset>                          // bitset
#include <concepts>                        // same_as
#include <cstddef>                         // size_t
#include <cstdint>                         // int8_t, uint8_t
#include <ranges>                          // iota
#include <stdexcept>                       // out_of_range

BOOST_AUTO_TEST_SUITE(BitFlagMapping)

namespace {

// A bitmask enumeration in the standard's style: each enumerator one bit, and no rank enumeration beside it.
enum class mode : std::uint8_t
{
        read  = 0x01,
        write = 0x02,
        exec  = 0x04,
        dir   = 0x08,
        link  = 0x10,
        sock  = 0x20,
};

// A signed underlying type, with one enumerator on its sign bit.
enum class signed_flag : std::int8_t
{
        low  = 0x01,
        mid  = 0x08,
        high = 0x40,
        sign = -0x80,
};

constexpr auto modes = std::array{mode::read, mode::write, mode::exec, mode::dir, mode::link, mode::sock};

} // namespace

// The universe defaults to every bit of the underlying type, and a narrower one is the second argument.
BOOST_AUTO_TEST_CASE(TheSizeDefaultsToTheUnderlyingTypesWidth)
{
        static_assert(xstd::bit_flag_mapping<mode>::size == 8UZ);
        static_assert(xstd::bit_flag_mapping<signed_flag>::size == 8UZ);
        static_assert(xstd::bit_flag_mapping<mode, 6UZ>::size == 6UZ);
        static_assert(std::same_as<decltype(xstd::bit_flag_mapping<mode>::size), std::size_t const>);
        static_assert(std::same_as<decltype(xstd::bit_flag_mapping<mode>::to_index(mode::read)), std::size_t>);
        static_assert(std::same_as<decltype(xstd::bit_flag_mapping<mode>::from_index(0UZ)), mode>);
        static_assert(noexcept(xstd::bit_flag_mapping<mode>::to_index(mode::read)));
        static_assert(noexcept(xstd::bit_flag_mapping<mode>::from_index(0UZ)));

        BOOST_CHECK(true);
}

// A one-bit value ranks at its bit's position, and each position below the size is the value with that bit alone.
BOOST_AUTO_TEST_CASE(AOneBitValueRanksAtItsBitsPosition)
{
        using mapping = xstd::bit_flag_mapping<mode>;
        for (auto const i : std::views::iota(0UZ, modes.size())) {
                BOOST_CHECK_EQUAL(mapping::to_index(modes[i]), i);
                BOOST_CHECK(mapping::from_index(i) == modes[i]);
        }
        for (auto const i : std::views::iota(0UZ, mapping::size)) {
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
        }
        static_assert(mapping::to_index(mode::sock) == 5UZ);
        static_assert(mapping::from_index(7UZ) == std::bit_cast<mode>(std::uint8_t{0x80}));
}

// The sign bit is a position like the others: the unsigned counterpart does the arithmetic.
BOOST_AUTO_TEST_CASE(TheSignBitIsTheHighestPosition)
{
        using mapping = xstd::bit_flag_mapping<signed_flag>;
        static_assert(mapping::to_index(signed_flag::low) == 0UZ);
        static_assert(mapping::to_index(signed_flag::mid) == 3UZ);
        static_assert(mapping::to_index(signed_flag::high) == 6UZ);
        static_assert(mapping::to_index(signed_flag::sign) == 7UZ);
        static_assert(mapping::from_index(7UZ) == signed_flag::sign);
        static_assert(mapping::from_index(6UZ) == signed_flag::high);
        for (auto const i : std::views::iota(0UZ, mapping::size)) {
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
        }
}

// A set keyed on the mask enumeration itself: iteration yields one-bit values, and a bit past the width is refused.
BOOST_AUTO_TEST_CASE(AMaskEnumerationKeysASetDirectly)
{
        using X      = xstd::basic_bit_fixed_set<mode, std::uint8_t, 6UZ, xstd::bit_flag_mapping<mode, 6UZ>>;
        auto const s = X{mode::exec, mode::read, mode::sock};
        BOOST_CHECK(std::ranges::equal(s, std::array{mode::read, mode::exec, mode::sock}));
        BOOST_CHECK(s.contains(mode::exec));
        BOOST_CHECK(not s.contains(mode::write));
        BOOST_CHECK(std::ranges::equal(~X(), modes));

        // 0x40 is one bit, but at position 6, which a width of 6 has no room for.
        auto x = s;
        BOOST_CHECK(not x.contains(std::bit_cast<mode>(std::uint8_t{0x40})));
        BOOST_CHECK_THROW(static_cast<void>(x.insert(std::bit_cast<mode>(std::uint8_t{0x40}))), std::out_of_range);
        BOOST_CHECK(x == s);

        // The signed one keys a set in the order of its positions, the sign bit last.
        using Y      = xstd::basic_bit_fixed_set<signed_flag, std::uint8_t, 8UZ, xstd::bit_flag_mapping<signed_flag>>;
        auto const y = Y(xstd::from_blocks, std::uint8_t{0x81});
        BOOST_CHECK(std::ranges::equal(y, std::array{signed_flag::low, signed_flag::sign}));
}

// A bitset as wide as a block is keyed the same way: a one-bit bitset ranks at its bit, and rank i is that bitset.
BOOST_AUTO_TEST_CASE(ABitsetRanksAtItsBitsPosition)
{
        using mapping = xstd::bit_flag_mapping<std::bitset<16>>;
        static_assert(xstd::sized_bit_index_mapping<mapping, std::bitset<16>> and mapping::size == 16UZ);
        static_assert(xstd::bit_flag_mapping<std::bitset<16>, 12UZ>::size == 12UZ);
        static_assert(mapping::to_index(std::bitset<16>(0x0400)) == 10UZ);
        static_assert(mapping::from_index(15UZ) == std::bitset<16>(0x8000));
        static_assert(not xstd::sized_bit_index_mapping<mapping, std::bitset<8>>);
        for (auto const i : std::views::iota(0UZ, mapping::size)) {
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
                BOOST_CHECK(mapping::from_index(i).count() == 1UZ and mapping::from_index(i).test(i));
        }
}

BOOST_AUTO_TEST_SUITE_END()
