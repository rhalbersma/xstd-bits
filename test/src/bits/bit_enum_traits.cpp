//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>            // day, letter, level, listed_enums, nine, perm, piece, sign, undeclared, wind
#include <xstd/bits/bit_enum_traits.hpp> // bit_enum_traits, enum_traits
#include <xstd/bits/bit_key_traits.hpp>  // bit_key_traits
#include <boost/test/unit_test.hpp>      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                      // derived_from, same_as
#include <cstddef>                       // size_t
#include <ranges>                        // iota, size
#include <utility>                       // to_underlying

BOOST_AUTO_TEST_SUITE(BitEnumTraits)

namespace {

// A requires-expression on a concrete type is ill-formed rather than false ([expr.prim.req]/5).
template<class E>
constexpr bool is_placed_in_bits = requires { xstd::bit_enum_traits<E>::size; };

template<class E>
constexpr bool has_default_key_traits = requires (E e) { xstd::bit_key_traits<E>::to_index(e); };

} // namespace

// The size is the count of values listed, whatever their spread: a gap costs no position.
BOOST_AUTO_TEST_CASE(TheSizeIsTheNumberOfListedValues)
{
        static_assert(xstd::bit_enum_traits<test::set::perm>::size == 3UZ);
        static_assert(xstd::bit_enum_traits<test::set::letter>::size == 3UZ);
        static_assert(xstd::bit_enum_traits<test::set::piece>::size == 6UZ);
        static_assert(xstd::bit_enum_traits<test::set::level>::size == 3UZ);
        static_assert(xstd::bit_enum_traits<test::set::sign>::size == 3UZ);
        static_assert(xstd::bit_enum_traits<test::set::day>::size == 5UZ);
        static_assert(xstd::bit_enum_traits<test::set::wind>::size == 8UZ);
        static_assert(xstd::bit_enum_traits<test::set::nine>::size == 9UZ);
        static_assert(std::same_as<decltype(xstd::bit_enum_traits<test::set::perm>::size), std::size_t const>);

        BOOST_CHECK(true);
}

// Each rank is the value's place in the list and back, in the order of the underlying values.
BOOST_AUTO_TEST_CASE_TEMPLATE(ARankIsTheValuesPlaceInTheList, E, test::set::listed_enums)
{
        using traits          = xstd::bit_enum_traits<E>;
        constexpr auto values = xstd::enum_traits<E>::values;
        static_assert(std::same_as<decltype(traits::to_index(values[0])), std::size_t>);
        static_assert(std::same_as<decltype(traits::from_index(0UZ)), E>);
        static_assert(noexcept(traits::to_index(values[0])));
        static_assert(noexcept(traits::from_index(0UZ)));
        static_assert(traits::size == std::ranges::size(values));

        for (auto const i : std::views::iota(0UZ, traits::size)) {
                BOOST_CHECK(traits::from_index(i) == values[i]);
                BOOST_CHECK_EQUAL(traits::to_index(values[i]), i);
                BOOST_CHECK(i == 0UZ or std::to_underlying(traits::from_index(i - 1UZ)) < std::to_underlying(traits::from_index(i)));
        }
}

// A value between, below or above the listed ones ranks at size or above, whether the list is dense or not.
BOOST_AUTO_TEST_CASE(AnUnlistedValueRanksAtTheSizeOrAbove)
{
        using test::set::letter;
        using test::set::level;
        using test::set::piece;
        using test::set::sign;
        static_assert(xstd::bit_enum_traits<piece>::to_index(static_cast<piece>(2)) == 6UZ);
        static_assert(xstd::bit_enum_traits<piece>::to_index(static_cast<piece>(0)) == 6UZ);
        static_assert(xstd::bit_enum_traits<sign>::to_index(static_cast<sign>(-1)) == 3UZ);
        static_assert(xstd::bit_enum_traits<letter>::to_index(static_cast<letter>(9)) >= 3UZ);
        static_assert(xstd::bit_enum_traits<letter>::to_index(static_cast<letter>(13)) >= 3UZ);
        static_assert(xstd::bit_enum_traits<level>::to_index(static_cast<level>(-301)) >= 3UZ);
        static_assert(xstd::bit_enum_traits<level>::to_index(static_cast<level>(-297)) >= 3UZ);
        static_assert(xstd::bit_enum_traits<piece>::to_index(static_cast<piece>(50)) == 6UZ);
        static_assert(xstd::bit_enum_traits<letter>::to_index(static_cast<letter>(200)) >= 3UZ);

        BOOST_CHECK(true);
}

// A listed enumeration is its own default key, and one listing nothing has no traits placing it in bits.
BOOST_AUTO_TEST_CASE(OnlyAListedEnumerationIsPlacedInBits)
{
        static_assert(std::derived_from<xstd::bit_key_traits<test::set::piece>, xstd::bit_enum_traits<test::set::piece>>);
        static_assert(xstd::bit_key_traits<test::set::piece>::size == 6UZ);
        static_assert(is_placed_in_bits<test::set::piece> and has_default_key_traits<test::set::piece>);
        static_assert(not is_placed_in_bits<test::set::undeclared>);
        static_assert(not has_default_key_traits<test::set::undeclared>);
        static_assert(not is_placed_in_bits<int>);

        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
