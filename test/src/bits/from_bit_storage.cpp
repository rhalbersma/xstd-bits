//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>               // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_array.hpp>        // basic_bit_array, bit_array
#include <xstd/bits/bit_static_set.hpp>   // basic_bit_static_set, bit_static_set
#include <xstd/bits/bit_vector.hpp>       // bit_vector
#include <xstd/bits/bitset.hpp>           // basic_bitset, bitset
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage, from_bit_storage_t
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                          // array
#include <bitset>                         // bitset
#include <concepts>                       // same_as
#include <cstddef>                        // size_t
#include <cstdint>                        // uint8_t, uint16_t, uint32_t, uint64_t
#include <type_traits>                    // is_constructible_v, is_default_constructible_v

BOOST_AUTO_TEST_SUITE(FromBitStorage)

namespace {

template<class T>
concept deduces_from_bit_storage_of = requires (T const& value) { xstd::basic_bit_array(xstd::from_bit_storage, value); };

template<class T>
concept bitset_deduces_from = requires (T const& value) { xstd::basic_bitset(value); };

// Declared only, for the concept below to call in an unevaluated operand.
template<class T>
[[maybe_unused]] auto from_braces(T)
        -> void;

template<class T>
concept braces_convert = requires { from_braces<T>({}); };

} // namespace

// The tag is std::from_range's twin: an explicit default constructor, so {} cannot stand in for it.
BOOST_AUTO_TEST_CASE(TheTagIsExplicitlyDefaultConstructible)
{
        static_assert(std::is_default_constructible_v<xstd::from_bit_storage_t>);
        static_assert(not braces_convert<xstd::from_bit_storage_t>);
        static_assert(std::same_as<decltype(xstd::from_bit_storage), xstd::from_bit_storage_t const>);
        BOOST_CHECK(true);
}

// An integer's digits are the width, and its value's bits are the positions, at every reading.
BOOST_AUTO_TEST_CASE(AnIntegerDeducesItsOwnWidth)
{
        constexpr auto word = std::uint16_t{0b1000'0000'0000'0101};

        constexpr auto a = xstd::basic_bit_array(xstd::from_bit_storage, word);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint16_t, 16> const>);
        static_assert(a == xstd::basic_bit_array<std::uint16_t, 16>(xstd::from_bit_storage, word));
        static_assert(a[0] and not a[1] and a[2] and a[15]);

        constexpr auto s = xstd::basic_bit_static_set(xstd::from_bit_storage, word);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_static_set<std::uint16_t, 16> const>);
        static_assert(s == xstd::basic_bit_static_set<std::uint16_t, 16>(xstd::from_bit_storage, word));
        static_assert(s.size() == 3UZ and s.contains(15UZ));

        constexpr auto b = xstd::basic_bitset(xstd::from_bit_storage, word);
        static_assert(std::same_as<decltype(b), xstd::basic_bitset<std::uint16_t, 16> const>);
        static_assert(b.count() == 3UZ and b.test(15UZ));
        BOOST_CHECK(a.to_bits<std::uint16_t>() == word);
}

// An array of blocks is its blocks' width, block i holding positions [i * digits, (i + 1) * digits).
BOOST_AUTO_TEST_CASE(AnArrayOfBlocksDeducesTheirWidth)
{
        constexpr auto blocks = std::array<std::uint8_t, 3>{0x01, 0x00, 0x80};

        constexpr auto a = xstd::basic_bit_array(xstd::from_bit_storage, blocks);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint8_t, 24> const>);
        static_assert(a[0] and a[23] and a.count() == 2UZ);

        constexpr auto s = xstd::basic_bit_static_set(xstd::from_bit_storage, blocks);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_static_set<std::uint8_t, 24> const>);
        static_assert(s.contains(0UZ) and s.contains(23UZ) and s.size() == 2UZ);

        constexpr auto b = xstd::basic_bitset(xstd::from_bit_storage, blocks);
        static_assert(std::same_as<decltype(b), xstd::basic_bitset<std::uint8_t, 24> const>);
        static_assert(b.test(23UZ));
        BOOST_CHECK((a == xstd::basic_bit_array<std::uint8_t, 24>(xstd::from_bit_storage, blocks)));
}

// The bitset's untagged guide is std::bitset's own integer constructor, at the width of the integer's type.
BOOST_AUTO_TEST_CASE(ABitsetDeducesItsWidthFromAnIntegersType)
{
        constexpr auto b8 = xstd::basic_bitset(std::uint8_t{0xA5});
        static_assert(std::same_as<decltype(b8), xstd::basic_bitset<std::size_t, 8> const>);
        static_assert(b8.count() == 4UZ and b8.test(7UZ));

        constexpr auto b64 = xstd::basic_bitset(0xFFFF'0000'0000'0000ULL);
        static_assert(std::same_as<decltype(b64), xstd::basic_bitset<std::size_t, 64> const>);
        static_assert(b64.count() == 16UZ);

        // Through the alias as well, where the alias exposes the width alone.
        constexpr auto b32 = xstd::bitset(std::uint32_t{1});
        static_assert(b32.size() == 32UZ);
        BOOST_CHECK(b8.to_ulong() == 0xA5UL);
}

// Signed integers are no field of bits, an empty array names no width, and neither does a run-time width.
BOOST_AUTO_TEST_CASE(OnlyAnUnsignedIntegerOrItsArrayDeduces)
{
        static_assert(deduces_from_bit_storage_of<std::uint64_t>);
        static_assert(deduces_from_bit_storage_of<std::array<std::uint32_t, 2>>);
        static_assert(not deduces_from_bit_storage_of<std::array<std::uint32_t, 0>>);
        static_assert(not deduces_from_bit_storage_of<int>);
        static_assert(not deduces_from_bit_storage_of<std::array<int, 2>>);
        static_assert(bitset_deduces_from<std::uint32_t>);
        static_assert(not bitset_deduces_from<int>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::uint64_t>);
#ifdef TEST_HAS_UINT128

        // Wider than unsigned long long, which std::bitset's integer constructor reads: only the tag reaches it.
        static_assert(not bitset_deduces_from<xstd::uint128>);
        constexpr auto wide = xstd::basic_bitset(xstd::from_bit_storage, xstd::uint128{1} << 100U);
        static_assert(std::same_as<decltype(wide), xstd::basic_bitset<xstd::uint128, 128> const>);
        static_assert(wide.test(100UZ) and wide.count() == 1UZ);

#endif
        BOOST_CHECK(true);
}

// Only what is bit storage is read through the tag: a std::bitset has bit storage and is not it, so it is cast.
BOOST_AUTO_TEST_CASE(OnlyWhatIsBitStorageIsReadThroughTheTag)
{
        static_assert(std::is_constructible_v<xstd::bit_static_set<64>, xstd::from_bit_storage_t, std::uint64_t>);
        static_assert(std::is_constructible_v<xstd::bit_static_set<64>, xstd::from_bit_storage_t, std::array<std::uint32_t, 2>>);
        static_assert(not std::is_constructible_v<xstd::bit_static_set<64>, xstd::from_bit_storage_t, std::bitset<64>>);
        static_assert(not std::is_constructible_v<xstd::bit_static_set<64>, xstd::from_bit_storage_t, xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
