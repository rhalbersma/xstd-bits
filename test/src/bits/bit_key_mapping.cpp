//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>             // all_block_types
#include <test/set/enums.hpp>               // day, letter, level, listed_enums, nine, perm, piece, sign, undeclared, wind
#include <test/set/strong_index.hpp>        // offset_mapping, strong_index
#include <xstd/bits/bit_fixed_set.hpp>      // basic_bit_fixed_set
#include <xstd/bits/bit_index_mapping.hpp>  // bit_index_mapping, sized_bit_index_mapping
#include <xstd/bits/bit_key_mapping.hpp>    // bit_find_mapping, bit_key_mapping, bit_range_mapping, enum_traits
#include <xstd/bits/detail/set_adaptor.hpp> // admits_width
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                        // min
#include <array>                            // array
#include <bit>                              // bit_cast
#include <concepts>                         // derived_from, same_as
#include <cstddef>                          // size_t
#include <cstdint>                          // int16_t, int64_t, int8_t, uint8_t
#include <iterator>                         // next
#include <limits>                           // numeric_limits
#include <ranges>                           // iota, size
#include <tuple>                            // tuple
#include <utility>                          // to_underlying

BOOST_AUTO_TEST_SUITE(BitKeyMapping)

namespace {

using identity = xstd::bit_key_mapping<std::size_t>;

template<class Key>
constexpr bool has_default_mapping = requires (Key key) { xstd::bit_key_mapping<Key>::to_index(key); };

// A key whose arithmetic is its underlying type's, in a range that starts below zero.
enum class storey : std::int8_t
{
        basement = -3,
        ground   = 0,
        roof     = 4
};

// Sorted, negative and gapped: an index by search rather than by subtraction.
constexpr auto ids = std::array{-40, 3, 17, 41, 1000};

// A key on an unsigned underlying type, in a range that starts above zero.
enum class channel : std::uint8_t
{
        first = 5,
        last  = 12
};

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
        static_assert(not xstd::sized_bit_index_mapping<identity, std::size_t>);
        static_assert(xstd::bits::detail::set::admits_width<identity, std::size_t, 0UZ>);
        static_assert(xstd::bits::detail::set::admits_width<identity, std::size_t, 100UZ>);

        BOOST_CHECK(true);
}

// Every unsigned integer is its own position, from the narrowest key to one wider than std::size_t.
BOOST_AUTO_TEST_CASE_TEMPLATE(AnUnsignedKeyIsItsOwnPosition, Key, test::all_block_types)
{
        using mapping = xstd::bit_key_mapping<Key>;
        static_assert(xstd::bit_index_mapping<mapping, Key> and not xstd::sized_bit_index_mapping<mapping, Key>);
        static_assert(std::same_as<decltype(mapping::to_index(Key())), std::size_t>);
        static_assert(std::same_as<decltype(mapping::from_index(0UZ)), Key>);
        static_assert(noexcept(mapping::to_index(Key())));
        static_assert(noexcept(mapping::from_index(0UZ)));
        static_assert(xstd::bits::detail::set::admits_width<mapping, Key, 100UZ>);

        // The largest key that names a position: the key's own maximum, or std::size_t's where the key is wider.
        constexpr auto top = std::numeric_limits<Key>::digits < std::numeric_limits<std::size_t>::digits ? static_cast<std::size_t>(std::numeric_limits<Key>::max()) : std::numeric_limits<std::size_t>::max();
        static_assert(mapping::to_index(mapping::from_index(top)) == top);
        static_assert(mapping::from_index(top) == static_cast<Key>(top));

        for (auto const i : std::views::iota(0UZ, std::min(top, 300UZ) + 1UZ)) {
                BOOST_CHECK(mapping::from_index(i) == static_cast<Key>(i));
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
                BOOST_CHECK(i == 0UZ or mapping::from_index(i - 1UZ) < mapping::from_index(i));
        }
}

using range_keys = std::tuple<std::int8_t, std::int16_t, int, std::int64_t, std::uint8_t, unsigned>;

// A range closes the universe at N keys from First, in order, from the type's most negative value or from any other.
BOOST_AUTO_TEST_CASE_TEMPLATE(ARangeMapsNKeysFromFirstOntoTheFirstNPositions, Key, range_keys)
{
        constexpr auto lowest = std::numeric_limits<Key>::min();
        using from_lowest     = xstd::bit_range_mapping<Key, lowest, 100UZ>;
        using from_middle     = xstd::bit_range_mapping<Key, Key(std::numeric_limits<Key>::is_signed ? -50 : 20), 100UZ>;
        static_assert(xstd::sized_bit_index_mapping<from_lowest, Key> and from_lowest::size == 100UZ);
        static_assert(xstd::bits::detail::set::admits_width<from_lowest, Key, 100UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<from_lowest, Key, 99UZ>);
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
BOOST_AUTO_TEST_CASE(ARangeFromTheMostNegativeValueSpansTheType)
{
        using bytes = xstd::bit_range_mapping<std::int8_t, std::numeric_limits<std::int8_t>::min(), 256UZ>;
        static_assert(bytes::to_index(-128) == 0UZ and bytes::to_index(0) == 128UZ and bytes::to_index(127) == 255UZ);
        static_assert(bytes::from_index(0UZ) == -128 and bytes::from_index(255UZ) == 127);

        using words = xstd::bit_range_mapping<std::int64_t, std::numeric_limits<std::int64_t>::min(), 64UZ>;
        static_assert(words::to_index(std::numeric_limits<std::int64_t>::min() + 63) == 63UZ);
        static_assert(words::from_index(63UZ) == std::numeric_limits<std::int64_t>::min() + 63);

        BOOST_CHECK(true);
}

// An enumeration counts in its underlying type: First may lie below zero, or above it on an unsigned type.
BOOST_AUTO_TEST_CASE(ARangeOfAnEnumerationCountsInItsUnderlyingType)
{
        using storeys = xstd::bit_range_mapping<storey, storey::basement, 8UZ>;
        static_assert(std::same_as<decltype(storeys::from_index(0UZ)), storey>);
        static_assert(noexcept(storeys::to_index(storey::ground)) and noexcept(storeys::from_index(0UZ)));
        static_assert(storeys::to_index(storey::basement) == 0UZ and storeys::to_index(storey::ground) == 3UZ and storeys::to_index(storey::roof) == 7UZ);
        static_assert(storeys::from_index(0UZ) == storey::basement and storeys::from_index(7UZ) == storey::roof);

        using channels = xstd::bit_range_mapping<channel, channel::first, 8UZ>;
        static_assert(channels::to_index(channel::first) == 0UZ and channels::to_index(channel::last) == 7UZ);
        static_assert(channels::from_index(7UZ) == channel::last);

        for (auto const i : std::views::iota(0UZ, storeys::size)) {
                BOOST_CHECK_EQUAL(storeys::to_index(storeys::from_index(i)), i);
                BOOST_CHECK(i == 0UZ or storeys::from_index(i - 1UZ) < storeys::from_index(i));
                BOOST_CHECK_EQUAL(channels::to_index(channels::from_index(i)), i);
        }

        // A set keyed on the range holds every key in it, in the enumeration's order.
        auto const s = xstd::basic_bit_fixed_set<storey, std::uint8_t, 8UZ, storeys>{storey::roof, storey::basement, storey::ground};
        BOOST_CHECK_EQUAL(s.size(), 3UZ);
        BOOST_CHECK(s.front() == storey::basement and s.back() == storey::roof);
        BOOST_CHECK(s.contains(storey::ground));
}

// A strong index type specializes the default, and round-trips through its position preserving order.
BOOST_AUTO_TEST_CASE(AStrongIndexSpecializesTheDefault)
{
        using mapping = xstd::bit_key_mapping<test::set::strong_index>;
        static_assert(not xstd::sized_bit_index_mapping<mapping, test::set::strong_index>);

        for (auto const i : std::views::iota(0UZ, 200UZ)) {
                BOOST_CHECK(mapping::from_index(i) == test::set::strong_index{.value = i});
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
                BOOST_CHECK((mapping::from_index(i) < mapping::from_index(i + 1UZ)));
        }
}

// A mapping of its own may close the universe with a size, and map a key to a position other than its value.
BOOST_AUTO_TEST_CASE(AMappingOfItsOwnMayCloseTheUniverse)
{
        using mapping = test::set::offset_mapping<10UZ, 5UZ>;
        static_assert(xstd::sized_bit_index_mapping<mapping, test::set::strong_index>);
        static_assert(mapping::size == 5UZ);

        // The owners' static_assert on the width: the size admits its own width and no other.
        static_assert(xstd::bits::detail::set::admits_width<mapping, test::set::strong_index, 5UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<mapping, test::set::strong_index, 4UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<mapping, test::set::strong_index, 6UZ>);

        BOOST_CHECK_EQUAL(mapping::to_index({.value = 10UZ}), 0UZ);
        BOOST_CHECK(mapping::from_index(4UZ) == test::set::strong_index{.value = 14UZ});
}

// An enumeration whose values are listed ranks by that list, closing the universe at its size; others have no default.
BOOST_AUTO_TEST_CASE(AListedEnumerationRanksByItsList)
{
        using mapping = xstd::bit_key_mapping<test::set::piece>;
        static_assert(xstd::sized_bit_index_mapping<mapping, test::set::piece> and mapping::size == 6UZ);
        static_assert(xstd::bits::detail::set::admits_width<mapping, test::set::piece, 6UZ>);
        static_assert(not xstd::bits::detail::set::admits_width<mapping, test::set::piece, 101UZ>);
        static_assert(mapping::to_index(test::set::piece::king) == 5UZ);
        static_assert(xstd::bit_key_mapping<test::set::perm>::from_index(2UZ) == test::set::perm::exec);
        static_assert(has_default_mapping<test::set::perm>);
        static_assert(not has_default_mapping<test::set::undeclared>);
        static_assert(not has_default_mapping<int>);

        BOOST_CHECK(true);
}

// The size is the count of values listed, whatever their spread: a gap costs no position.
BOOST_AUTO_TEST_CASE(TheSizeIsTheNumberOfListedValues)
{
        static_assert(xstd::bit_key_mapping<test::set::perm>::size == 3UZ);
        static_assert(xstd::bit_key_mapping<test::set::letter>::size == 3UZ);
        static_assert(xstd::bit_key_mapping<test::set::piece>::size == 6UZ);
        static_assert(xstd::bit_key_mapping<test::set::level>::size == 3UZ);
        static_assert(xstd::bit_key_mapping<test::set::sign>::size == 3UZ);
        static_assert(xstd::bit_key_mapping<test::set::day>::size == 5UZ);
        static_assert(xstd::bit_key_mapping<test::set::wind>::size == 8UZ);
        static_assert(xstd::bit_key_mapping<test::set::nine>::size == 9UZ);
        static_assert(std::same_as<decltype(xstd::bit_key_mapping<test::set::perm>::size), std::size_t const>);
        static_assert(std::same_as<decltype(xstd::bit_key_mapping<test::set::piece>::size), std::size_t const>);

        BOOST_CHECK(true);
}

// Values that run without a gap are a range from the first, and scattered ones are a search of the list.
BOOST_AUTO_TEST_CASE(ConsecutiveValuesAreARangeAndScatteredOnesASearch)
{
        using test::set::letter;
        using test::set::perm;
        using test::set::piece;
        using test::set::sign;
        static_assert(std::derived_from<xstd::bit_key_mapping<perm>, xstd::bit_range_mapping<perm, perm::read, 3UZ>>);
        static_assert(std::derived_from<xstd::bit_key_mapping<letter>, xstd::bit_range_mapping<letter, letter::a, 3UZ>>);
        static_assert(std::derived_from<xstd::bit_key_mapping<piece>, xstd::bit_find_mapping<piece, xstd::enum_traits<piece>::values>>);
        static_assert(std::derived_from<xstd::bit_key_mapping<sign>, xstd::bit_find_mapping<sign, xstd::enum_traits<sign>::values>>);

        BOOST_CHECK(true);
}

// Each rank is the value's place in the list and back, in the order of the underlying values.
BOOST_AUTO_TEST_CASE_TEMPLATE(ARankIsTheValuesPlaceInTheList, E, test::set::listed_enums)
{
        using mapping         = xstd::bit_key_mapping<E>;
        constexpr auto values = xstd::enum_traits<E>::values;
        static_assert(xstd::sized_bit_index_mapping<mapping, E>);
        static_assert(noexcept(mapping::to_index(values[0])));
        static_assert(noexcept(mapping::from_index(0UZ)));
        static_assert(mapping::size == std::ranges::size(values));

        for (auto const i : std::views::iota(0UZ, mapping::size)) {
                BOOST_CHECK(mapping::from_index(i) == values[i]);
                BOOST_CHECK_EQUAL(mapping::to_index(values[i]), i);
                BOOST_CHECK(i == 0UZ or std::to_underlying(mapping::from_index(i - 1UZ)) < std::to_underlying(mapping::from_index(i)));
        }
}

// A value between, below or above the listed ones ranks at size or above, whether the list is dense or not.
BOOST_AUTO_TEST_CASE(AnUnlistedValueRanksAtTheSizeOrAbove)
{
        using test::set::letter;
        using test::set::level;
        using test::set::piece;
        using test::set::sign;
        static_assert(xstd::bit_key_mapping<piece>::to_index(static_cast<piece>(2)) == 6UZ);
        static_assert(xstd::bit_key_mapping<piece>::to_index(static_cast<piece>(0)) == 6UZ);
        static_assert(xstd::bit_key_mapping<sign>::to_index(static_cast<sign>(-1)) == 3UZ);
        static_assert(xstd::bit_key_mapping<letter>::to_index(static_cast<letter>(9)) >= 3UZ);
        static_assert(xstd::bit_key_mapping<letter>::to_index(static_cast<letter>(13)) >= 3UZ);
        static_assert(xstd::bit_key_mapping<level>::to_index(static_cast<level>(-301)) >= 3UZ);
        static_assert(xstd::bit_key_mapping<level>::to_index(static_cast<level>(-297)) >= 3UZ);
        static_assert(xstd::bit_key_mapping<piece>::to_index(static_cast<piece>(50)) == 6UZ);
        static_assert(xstd::bit_key_mapping<piece>::to_index(static_cast<piece>(101)) == 6UZ);
        static_assert(xstd::bit_key_mapping<letter>::to_index(static_cast<letter>(200)) >= 3UZ);

        // At run time too, for the search's two ways of missing: past the last key and between two.
        BOOST_CHECK_EQUAL(xstd::bit_key_mapping<piece>::to_index(std::bit_cast<piece>(std::uint8_t{101})), 6UZ);
        BOOST_CHECK_EQUAL(xstd::bit_key_mapping<piece>::to_index(std::bit_cast<piece>(std::uint8_t{2})), 6UZ);
}

// Sparse integer identifiers, each at its rank in the sorted list, key a set of as many positions as there are ids.
BOOST_AUTO_TEST_CASE(SparseIdentifiersKeyASetThroughASearch)
{
        using mapping = xstd::bit_find_mapping<int, ids>;
        static_assert(xstd::sized_bit_index_mapping<mapping, int> and mapping::size == 5UZ);
        static_assert(mapping::to_index(-40) == 0UZ and mapping::to_index(1000) == 4UZ);
        static_assert(mapping::from_index(2UZ) == 17 and mapping::to_index(18) == 5UZ);

        auto s = xstd::basic_bit_fixed_set<int, std::uint8_t, 5UZ, mapping>{1000, 17, -40};
        BOOST_CHECK_EQUAL(s.size(), 3UZ);
        BOOST_CHECK(s.front() == -40 and s.back() == 1000);
        BOOST_CHECK(s.contains(17) and not s.contains(3));
        s.insert(3);
        BOOST_CHECK(*std::next(s.begin()) == 3);
        for (auto const i : std::views::iota(0UZ, mapping::size)) {
                BOOST_CHECK_EQUAL(mapping::to_index(mapping::from_index(i)), i);
        }
}

BOOST_AUTO_TEST_SUITE_END()
