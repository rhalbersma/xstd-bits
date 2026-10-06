//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>             // all_block_types
#include <test/set/enums.hpp>               // perm, piece, undeclared
#include <test/set/strong_index.hpp>        // offset_traits, strong_index
#include <xstd/bits/bit_enum_traits.hpp>    // bit_enum_traits
#include <xstd/bits/bit_key_traits.hpp>     // bit_key_traits
#include <xstd/bits/detail/set_adaptor.hpp> // admits_width
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <algorithm>                        // min
#include <concepts>                         // derived_from, same_as
#include <cstddef>                          // size_t
#include <cstdint>                          // int16_t, int64_t, int8_t, uint8_t
#include <limits>                           // numeric_limits
#include <ranges>                           // iota
#include <tuple>                            // tuple

BOOST_AUTO_TEST_SUITE(BitKeyTraits)

namespace {

// A requires-expression on a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class KeyTraits>
constexpr bool has_size = requires { KeyTraits::size; };

using identity = xstd::bit_key_traits<std::size_t>;

template<class Key>
constexpr bool has_default_traits = requires (Key key) { xstd::bit_key_traits<Key>::to_index(key); };

} // namespace

BOOST_AUTO_TEST_CASE(AStdSizeTKeyIsItsOwnPosition)
{
        static_assert(identity::to_index(0UZ) == 0UZ);
        static_assert(identity::from_index(0UZ) == 0UZ);
        static_assert(identity::to_index(std::numeric_limits<std::size_t>::max()) == std::numeric_limits<std::size_t>::max());
        static_assert(std::same_as<decltype(identity::from_index(0UZ)), std::size_t>);
        static_assert(noexcept(identity::to_index(0UZ)));
        static_assert(noexcept(identity::from_index(0UZ)));

        for (auto const i : std::views::iota(0UZ, 200UZ)) {
                BOOST_CHECK_EQUAL(identity::to_index(i), i);
                BOOST_CHECK_EQUAL(identity::from_index(i), i);
        }
}

// No size: a std::size_t key leaves the universe open, so an owner of any width or capacity takes it.
BOOST_AUTO_TEST_CASE(AStdSizeTKeyLeavesTheUniverseOpen)
{
        static_assert(not has_size<identity>);
        static_assert(xstd::bits::detail::set::admits_width<identity, 0UZ>);
        static_assert(xstd::bits::detail::set::admits_width<identity, 100UZ>);

        BOOST_CHECK(true);
}

// Every unsigned integer is its own position, from the narrowest key to one wider than std::size_t.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnUnsignedKeyIsItsOwnPosition, Key, test::all_block_types)
{
        using traits = xstd::bit_key_traits<Key>;
        static_assert(not has_size<traits>);
        static_assert(std::same_as<decltype(traits::to_index(Key())), std::size_t>);
        static_assert(std::same_as<decltype(traits::from_index(0UZ)), Key>);
        static_assert(noexcept(traits::to_index(Key())));
        static_assert(noexcept(traits::from_index(0UZ)));
        static_assert(xstd::bits::detail::set::admits_width<traits, 100UZ>);

        // The largest key that names a position: the key's own maximum, or std::size_t's where the key is wider.
        constexpr auto top = std::numeric_limits<Key>::digits < std::numeric_limits<std::size_t>::digits ? static_cast<std::size_t>(std::numeric_limits<Key>::max()) : std::numeric_limits<std::size_t>::max();
        static_assert(traits::to_index(traits::from_index(top)) == top);
        static_assert(traits::from_index(top) == static_cast<Key>(top));

        for (auto const i : std::views::iota(0UZ, std::min(top, 300UZ) + 1UZ)) {
                BOOST_CHECK(traits::from_index(i) == static_cast<Key>(i));
                BOOST_CHECK_EQUAL(traits::to_index(traits::from_index(i)), i);
                BOOST_CHECK(i == 0UZ or traits::from_index(i - 1UZ) < traits::from_index(i));
        }
}

using offset_keys = std::tuple<std::int8_t, std::int16_t, int, std::int64_t, std::uint8_t, unsigned>;

// An offset closes the universe at N keys from First, in order, from the type's most negative value or from any other.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnOffsetMapsNKeysFromFirstOntoTheFirstNPositions, Key, offset_keys)
{
        constexpr auto lowest = std::numeric_limits<Key>::min();
        using from_lowest     = xstd::bit_offset_traits<Key, lowest, 100UZ>;
        using from_middle     = xstd::bit_offset_traits<Key, Key(std::numeric_limits<Key>::is_signed ? -50 : 20), 100UZ>;
        static_assert(has_size<from_lowest> and from_lowest::size == 100UZ);
        static_assert(xstd::bits::detail::set::admits_width<from_lowest, 100UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<from_lowest, 99UZ>);
        static_assert(std::same_as<decltype(from_lowest::to_index(Key())), std::size_t>);
        static_assert(std::same_as<decltype(from_lowest::from_index(0UZ)), Key>);
        static_assert(noexcept(from_lowest::to_index(Key())) and noexcept(from_lowest::from_index(0UZ)));
        static_assert(from_lowest::to_index(lowest) == 0UZ);
        static_assert(from_lowest::from_index(99UZ) == static_cast<Key>(lowest + 99));

        for (auto const i : std::views::iota(0UZ, 100UZ)) {
                BOOST_CHECK_EQUAL(from_lowest::to_index(from_lowest::from_index(i)), i);
                BOOST_CHECK_EQUAL(from_middle::to_index(from_middle::from_index(i)), i);
                BOOST_CHECK(i == 0UZ or from_middle::from_index(i - 1UZ) < from_middle::from_index(i));
        }
        BOOST_CHECK(from_middle::from_index(0UZ) == Key(std::numeric_limits<Key>::is_signed ? -50 : 20));
}

// The whole of a narrow signed type, and the far end of the widest: the distance from First stays exact.
BOOST_AUTO_TEST_CASE(AnOffsetFromTheMostNegativeValueSpansTheType)
{
        using bytes = xstd::bit_offset_traits<std::int8_t, std::numeric_limits<std::int8_t>::min(), 256UZ>;
        static_assert(bytes::to_index(-128) == 0UZ and bytes::to_index(0) == 128UZ and bytes::to_index(127) == 255UZ);
        static_assert(bytes::from_index(0UZ) == -128 and bytes::from_index(255UZ) == 127);

        using words = xstd::bit_offset_traits<std::int64_t, std::numeric_limits<std::int64_t>::min(), 64UZ>;
        static_assert(words::to_index(std::numeric_limits<std::int64_t>::min() + 63) == 63UZ);
        static_assert(words::from_index(63UZ) == std::numeric_limits<std::int64_t>::min() + 63);

        BOOST_CHECK(true);
}

// A strong index type specializes the default, and round-trips through its position preserving order.
BOOST_AUTO_TEST_CASE(AStrongIndexSpecializesTheDefault)
{
        using traits = xstd::bit_key_traits<test::set::strong_index>;
        static_assert(not has_size<traits>);

        for (auto const i : std::views::iota(0UZ, 200UZ)) {
                BOOST_CHECK(traits::from_index(i) == test::set::strong_index{.value = i});
                BOOST_CHECK_EQUAL(traits::to_index(traits::from_index(i)), i);
                BOOST_CHECK((traits::from_index(i) < traits::from_index(i + 1UZ)));
        }
}

// A traits type of its own may close the universe with a size, and map a key to a position other than its value.
BOOST_AUTO_TEST_CASE(ATraitsTypeOfItsOwnMayCloseTheUniverse)
{
        using traits = test::set::offset_traits<10UZ, 5UZ>;
        static_assert(has_size<traits>);
        static_assert(traits::size == 5UZ);

        // The owners' static_assert on the width: the size admits its own width and no other.
        static_assert(xstd::bits::detail::set::admits_width<traits, 5UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<traits, 4UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<traits, 6UZ>);

        BOOST_CHECK_EQUAL(traits::to_index({.value = 10UZ}), 0UZ);
        BOOST_CHECK(traits::from_index(4UZ) == test::set::strong_index{.value = 14UZ});
}

// An enumeration whose values are listed ranks by that list, closing the universe at its size; others have no default.
BOOST_AUTO_TEST_CASE(AListedEnumerationRanksByItsList)
{
        using traits = xstd::bit_key_traits<test::set::piece>;
        static_assert(std::derived_from<traits, xstd::bit_enum_traits<test::set::piece>>);
        static_assert(has_size<traits> and traits::size == 6UZ);
        static_assert(xstd::bits::detail::set::admits_width<traits, 6UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<traits, 101UZ>);
        static_assert(traits::to_index(test::set::piece::king) == 5UZ);
        static_assert(xstd::bit_key_traits<test::set::perm>::from_index(2UZ) == test::set::perm::exec);
        static_assert(has_default_traits<test::set::perm>);
        static_assert(not has_default_traits<test::set::undeclared>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
