//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>               // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit/bit_convert.hpp>  // bit_convert
#include <xstd/bits/bit_array.hpp>        // basic_bit_array, bit_array
#include <xstd/bits/bit_fixed_set.hpp>    // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_vector.hpp>       // bit_vector
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage, from_bit_storage_t
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                          // array
#include <bitset>                         // bitset
#include <concepts>                       // same_as
#include <cstdint>                        // uint8_t, uint16_t, uint32_t, uint64_t
#include <type_traits>                    // is_constructible_v, is_default_constructible_v

BOOST_AUTO_TEST_SUITE(FromBitStorage)

namespace {

template<class T>
concept deduces_from_bit_storage_of = requires (T const& value) { xstd::basic_bit_array(xstd::from_bit_storage, value); };

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
        constexpr auto block = std::uint16_t{0b1000'0000'0000'0101};

        constexpr auto a = xstd::basic_bit_array(xstd::from_bit_storage, block);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint16_t, 16> const>);
        static_assert(a == xstd::basic_bit_array<std::uint16_t, 16>(xstd::from_bit_storage, block));
        static_assert(a[0] and not a[1] and a[2] and a[15]);

        constexpr auto s = xstd::basic_bit_fixed_set(xstd::from_bit_storage, block);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_fixed_set<std::uint16_t, 16> const>);
        static_assert(s == xstd::basic_bit_fixed_set<std::uint16_t, 16>(xstd::from_bit_storage, block));
        static_assert(s.size() == 3UZ and s.contains(15UZ));
        BOOST_CHECK(xstd::bit_convert<std::uint16_t>(a) == block);
}

// An array of blocks is its blocks' width, block i holding positions [i * digits, (i + 1) * digits).
BOOST_AUTO_TEST_CASE(AnArrayOfBlocksDeducesTheirWidth)
{
        constexpr auto blocks = std::array<std::uint8_t, 3>{0x01, 0x00, 0x80};

        constexpr auto a = xstd::basic_bit_array(xstd::from_bit_storage, blocks);
        static_assert(std::same_as<decltype(a), xstd::basic_bit_array<std::uint8_t, 24> const>);
        static_assert(a[0] and a[23] and a.count() == 2UZ);

        constexpr auto s = xstd::basic_bit_fixed_set(xstd::from_bit_storage, blocks);
        static_assert(std::same_as<decltype(s), xstd::basic_bit_fixed_set<std::uint8_t, 24> const>);
        static_assert(s.contains(0UZ) and s.contains(23UZ) and s.size() == 2UZ);
        BOOST_CHECK((a == xstd::basic_bit_array<std::uint8_t, 24>(xstd::from_bit_storage, blocks)));
}

// Signed integers are no field of bits, an empty array names no width, and neither does a run-time width.
BOOST_AUTO_TEST_CASE(OnlyAnUnsignedIntegerOrItsArrayDeduces)
{
        static_assert(deduces_from_bit_storage_of<std::uint64_t>);
        static_assert(deduces_from_bit_storage_of<std::array<std::uint32_t, 2>>);
        static_assert(not deduces_from_bit_storage_of<std::array<std::uint32_t, 0>>);
        static_assert(not deduces_from_bit_storage_of<int>);
        static_assert(not deduces_from_bit_storage_of<std::array<int, 2>>);
        static_assert(not std::is_constructible_v<xstd::bit_vector, xstd::from_bit_storage_t, std::uint64_t>);
#ifdef TEST_HAS_UINT128

        // Wider than unsigned long long, and still a field of bits the tag reads.
        static_assert(deduces_from_bit_storage_of<xstd::uint128>);
        constexpr auto wide = xstd::basic_bit_array(xstd::from_bit_storage, xstd::uint128{1} << 100U);
        static_assert(std::same_as<decltype(wide), xstd::basic_bit_array<xstd::uint128, 128> const>);
        static_assert(wide[100] and wide.count() == 1UZ);

#endif
        BOOST_CHECK(true);
}

// Only what is bit storage is read through the tag: a std::bitset has bit storage and is not it, so it is converted.
BOOST_AUTO_TEST_CASE(OnlyWhatIsBitStorageIsReadThroughTheTag)
{
        static_assert(std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_bit_storage_t, std::uint64_t>);
        static_assert(std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_bit_storage_t, std::array<std::uint32_t, 2>>);
        static_assert(not std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_bit_storage_t, std::bitset<64>>);
        static_assert(not std::is_constructible_v<xstd::bit_fixed_set<64>, xstd::from_bit_storage_t, xstd::bit_array<64>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
