//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/lookup.hpp>                                // lookup_mismatches
#include <xstd/bits/bit_concepts/bit_mask_mapping.hpp>        // bit_mask_mapping
#include <xstd/bits/bit_concepts/sized_bit_index_mapping.hpp> // sized_bit_index_mapping
#include <xstd/bits/bit_fixed_set.hpp>                        // basic_bit_fixed_set
#include <xstd/bits/bit_flag_mapping.hpp>                     // bit_flag_mapping
#include <xstd/bits/from_blocks.hpp>                          // from_blocks
#include <boost/test/unit_test.hpp>                           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <algorithm>                                          // ranges::equal
#include <array>                                              // array
#include <bit>                                                // bit_cast, has_single_bit
#include <bitset>                                             // bitset
#include <concepts>                                           // same_as
#include <cstddef>                                            // size_t
#include <cstdint>                                            // int8_t, uint16_t, uint64_t, uint8_t
#include <functional>                                         // greater
#include <limits>                                             // numeric_limits
#include <ranges>                                             // iota
#include <set>                                                // set
#include <stdexcept>                                          // out_of_range

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

// The bytes whose key-ness the mapping misjudges: a key has one bit set, below N.
template<std::size_t N>
auto mode_mismatches()
        -> std::size_t
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 256U)) {
                auto const key = std::has_single_bit(word) and word < (1U << N);
                mismatches += static_cast<std::size_t>(xstd::bit_flag_mapping<mode, N>::is_key(std::bit_cast<mode>(static_cast<std::uint8_t>(word))) != key);
        }
        return mismatches;
}

// The same over every 16-bit bitset, its count() the bits it has set.
template<std::size_t N>
auto bitset_mismatches()
        -> std::size_t
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0UZ, 1UZ << 16UZ)) {
                auto const b   = std::bitset<16>(word);
                auto const key = b.count() == 1UZ and word < (1UZ << N);
                mismatches += static_cast<std::size_t>(xstd::bit_flag_mapping<std::bitset<16>, N>::is_key(b) != key);
        }
        return mismatches;
}

// Every subset of a set of modes against std::set, asked every byte, the multi-bit and zero values among them.
template<class X, class Model>
auto mode_lookup_mismatches()
        -> std::size_t
{
        auto mismatches = 0UZ;
        for (auto const mask : std::views::iota(0UZ, 1UZ << modes.size())) {
                auto a     = X();
                auto model = Model();
                for (auto const i : std::views::iota(0UZ, modes.size())) {
                        if (((mask >> i) & 1UZ) != 0UZ) {
                                a.insert(modes[i]);
                                model.insert(modes[i]);
                        }
                }
                for (auto const word : std::views::iota(0U, 256U)) {
                        mismatches += test::set::lookup_mismatches(a, model, std::bit_cast<mode>(static_cast<std::uint8_t>(word)));
                }
        }
        return mismatches;
}

// The bytes whose key-ness an 8-bit integer's mapping misjudges: one bit set, below N, and never the sign bit.
template<class Mask, std::size_t N>
auto integer_mismatches()
        -> std::size_t
{
        auto mismatches = 0UZ;
        for (auto const word : std::views::iota(0U, 256U)) {
                auto const key = std::has_single_bit(word) and word < (1U << N);
                mismatches += static_cast<std::size_t>(xstd::bit_flag_mapping<Mask, N>::is_key(static_cast<Mask>(word)) != key);
        }
        return mismatches;
}

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

// A key has exactly one bit set, below N: zero, a value of several bits, or one bit at or above N is none.
BOOST_AUTO_TEST_CASE(AKeyIsAOneBitValueBelowTheSize)
{
        static_assert(std::same_as<decltype(xstd::bit_flag_mapping<mode>::is_key(mode::read)), bool>);
        static_assert(noexcept(xstd::bit_flag_mapping<mode>::is_key(mode::read)));
        static_assert(xstd::bit_flag_mapping<mode>::is_key(mode::sock) and not xstd::bit_flag_mapping<mode>::is_key(std::bit_cast<mode>(std::uint8_t{0})));
        BOOST_CHECK_EQUAL(mode_mismatches<8UZ>(), 0UZ);
        BOOST_CHECK_EQUAL(mode_mismatches<6UZ>(), 0UZ);
        BOOST_CHECK_EQUAL(mode_mismatches<1UZ>(), 0UZ);

        // The sign bit is one bit like the others, and the value with every bit set is none.
        BOOST_CHECK(xstd::bit_flag_mapping<signed_flag>::is_key(signed_flag::sign));
        BOOST_CHECK(not xstd::bit_flag_mapping<signed_flag>::is_key(std::bit_cast<signed_flag>(std::int8_t{-1})));
}

// A bitset likewise: count() is 1, at a position below N.
BOOST_AUTO_TEST_CASE(ABitsetKeyHasOneBitBelowTheSize)
{
        BOOST_CHECK_EQUAL(bitset_mismatches<16UZ>(), 0UZ);
        BOOST_CHECK_EQUAL(bitset_mismatches<12UZ>(), 0UZ);
}

// A value that is no key is no element of a set of modes, which bounds it among the keys in either direction.
BOOST_AUTO_TEST_CASE(AValueThatIsNoKeyIsNoElement)
{
        using ascending  = xstd::basic_bit_fixed_set<mode, std::uint8_t, 6UZ, xstd::bit_flag_mapping<mode, 6UZ>>;
        using descending = xstd::basic_bit_fixed_set<mode, std::uint8_t, 6UZ, xstd::bit_flag_mapping<mode, 6UZ>, std::greater<>>;
        BOOST_CHECK_EQUAL((mode_lookup_mismatches<ascending, std::set<mode>>()), 0UZ);
        BOOST_CHECK_EQUAL((mode_lookup_mismatches<descending, std::set<mode, std::greater<>>>()), 0UZ);

        // read | write lies between write and exec, and the set erases neither for it.
        auto s                = ascending{mode::read, mode::write, mode::exec};
        auto const read_write = std::bit_cast<mode>(std::uint8_t{0x03});
        BOOST_CHECK(not s.contains(read_write) and s.count(read_write) == 0UZ); // NOLINT(readability-container-contains): count is the member under test
        BOOST_CHECK(s.find(read_write) == s.end());                             // NOLINT(readability-container-contains): find is the member under test
        BOOST_CHECK(*s.lower_bound(read_write) == mode::exec and *s.upper_bound(read_write) == mode::exec);
        BOOST_CHECK_EQUAL(s.erase(read_write), 0UZ);
        BOOST_CHECK_EQUAL(s.size(), 3UZ);
}

// An integer is keyed the same way, its unsigned counterpart doing the arithmetic, and a signed one below its sign bit.
BOOST_AUTO_TEST_CASE(AnIntegerRanksAtItsBitsPositionBelowItsSignBit)
{
        static_assert(xstd::bit_flag_mapping<std::uint8_t>::size == 8UZ and xstd::bit_flag_mapping<std::uint64_t>::size == 64UZ);
        static_assert(xstd::bit_flag_mapping<std::int8_t>::size == 7UZ and xstd::bit_flag_mapping<int>::size == 31UZ);
        static_assert(std::same_as<xstd::bit_flag_mapping<int>::block_type, unsigned> and std::same_as<xstd::bit_flag_mapping<std::int8_t>::block_type, std::uint8_t>);
        static_assert(xstd::bit_mask_mapping<xstd::bit_flag_mapping<int>, int> and xstd::bit_mask_mapping<xstd::bit_flag_mapping<std::uint16_t, 12UZ>, std::uint16_t>);
        static_assert(xstd::bit_flag_mapping<int>::to_index(0x4000'0000) == 30UZ and xstd::bit_flag_mapping<int>::from_index(30UZ) == 0x4000'0000);
        static_assert(xstd::bit_flag_mapping<std::uint64_t>::from_index(63UZ) == 0x8000'0000'0000'0000ULL);
        static_assert(xstd::bit_flag_mapping<int>::to_block(-1) == 0xFFFF'FFFFU and xstd::bit_flag_mapping<int>::from_block(0x7FFF'FFFFU) == 0x7FFF'FFFF);
        for (auto const i : std::views::iota(0UZ, xstd::bit_flag_mapping<std::int8_t>::size)) {
                BOOST_CHECK_EQUAL(xstd::bit_flag_mapping<std::int8_t>::to_index(xstd::bit_flag_mapping<std::int8_t>::from_index(i)), i);
                BOOST_CHECK(xstd::bit_flag_mapping<std::int8_t>::from_index(i) > 0);
        }
        BOOST_CHECK_EQUAL((integer_mismatches<std::uint8_t, 8UZ>()), 0UZ);
        BOOST_CHECK_EQUAL((integer_mismatches<std::uint8_t, 5UZ>()), 0UZ);
        BOOST_CHECK_EQUAL((integer_mismatches<std::int8_t, 7UZ>()), 0UZ);
        BOOST_CHECK(not xstd::bit_flag_mapping<int>::is_key(std::numeric_limits<int>::min()) and not xstd::bit_flag_mapping<int>::is_key(-1));
}

BOOST_AUTO_TEST_SUITE_END()
