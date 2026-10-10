//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit/bit_convert.hpp>          // bit_convert
#include <xstd/bits/bit_array.hpp>                // basic_bit_array, bit_array
#include <xstd/bits/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_vector.hpp>               // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/dynamic_bitset.hpp> // bit_convert
#include <boost/dynamic_bitset.hpp>               // dynamic_bitset
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <bitset>                                 // bitset
#include <cstddef>                                // size_t
#include <cstdint>                                // uint8_t
#include <ranges>                                 // iota
#include <utility>                                // declval

BOOST_AUTO_TEST_SUITE(Shifts)

namespace {

// A pattern with no period a shift could hide behind.
[[nodiscard]] constexpr auto in_pattern(std::size_t i) noexcept
        -> bool
{
        return (i * 7UZ + 3UZ) % 5UZ < 2UZ;
}

// Every shift from nought to past the width, both directions and both forms, against std::bitset of the same width.
template<class Array, std::size_t N>
auto check_as_bitset()
        -> void
{
        auto bits = std::bitset<N>();
        for (auto const i : std::views::iota(0UZ, N)) {
                bits[i] = in_pattern(i);
        }
        auto const a = xstd::bit_convert<Array>(bits);
        for (auto const n : std::views::iota(0UZ, N + 3UZ)) {
                BOOST_CHECK(xstd::bit_convert<std::bitset<N>>(a << n) == (bits << n));
                BOOST_CHECK(xstd::bit_convert<std::bitset<N>>(a >> n) == (bits >> n));

                auto left = a;
                left <<= n;
                BOOST_CHECK(left == (a << n));

                auto right = a;
                right >>= n;
                BOOST_CHECK(right == (a >> n));

                // A whole view shifts the bits it views, as the owner does.
                auto through = a;
                auto view    = xstd::bit_span(through);
                view <<= n;
                BOOST_CHECK(through == left);
        }
}

// The same against boost::dynamic_bitset, whose shifts keep the size as these do.
template<class Vector>
auto check_as_dynamic_bitset(std::size_t size)
        -> void
{
        auto bits = boost::dynamic_bitset<>(size);
        for (auto const i : std::views::iota(0UZ, size)) {
                bits[i] = in_pattern(i);
        }
        auto const v = xstd::bit_convert<Vector>(bits);
        for (auto const n : std::views::iota(0UZ, size + 3UZ)) {
                auto left = v;
                left <<= n;
                BOOST_CHECK(xstd::bit_convert<boost::dynamic_bitset<>>(left) == (bits << n));
                BOOST_CHECK_EQUAL(left.size(), size);

                auto right = v;
                right >>= n;
                BOOST_CHECK(xstd::bit_convert<boost::dynamic_bitset<>>(right) == (bits >> n));
        }
}

template<class X>
concept shifts_in_place = requires (X x) { x <<= 1UZ; x >>= 1UZ; };

template<class X>
concept shifts_by_value = requires (X const x) { x << 1UZ; x >> 1UZ; };

} // namespace

// Position i moves to i + n under <<, as std::bitset's does: in index order that is std::shift_right, zeros filling in.
BOOST_AUTO_TEST_CASE(AFixedWidthShiftsAsStdBitset)
{
        check_as_bitset<xstd::bit_array<0>, 0>();
        check_as_bitset<xstd::basic_bit_array<std::uint8_t, 13>, 13>();
        check_as_bitset<xstd::bit_array<64>, 64>();
        check_as_bitset<xstd::basic_bit_array<std::uint8_t, 130>, 130>();
        check_as_bitset<xstd::bit_array<130>, 130>();

        static_assert([] -> bool {
                auto a = xstd::basic_bit_array<std::uint8_t, 13>();
                a[1]   = true;
                a <<= 3;
                return a[4] and a.count() == 1UZ and (a >> 4)[0] and (a << 13).none();
        }());
}

// A run-time width keeps its size: what passes the end is dropped, as boost::dynamic_bitset's shifts drop it.
BOOST_AUTO_TEST_CASE(ARunTimeWidthShiftsAsDynamicBitset)
{
        for (auto const size : {0UZ, 1UZ, 7UZ, 8UZ, 63UZ, 64UZ, 65UZ, 200UZ}) {
                check_as_dynamic_bitset<xstd::bit_vector>(size);
                check_as_dynamic_bitset<xstd::basic_bit_vector<std::uint8_t>>(size);
        }
}

// A whole view shifts what it views, a view of const bits nothing, and the value forms are an owner's alone.
BOOST_AUTO_TEST_CASE(OnlyWhatOwnsOrWritesAWholeRowShifts)
{
        using array = xstd::bit_array<40>;
        using span  = decltype(xstd::bit_span(std::declval<array&>()));
        static_assert(shifts_in_place<array> and shifts_in_place<xstd::bit_vector> and shifts_in_place<span>);
        static_assert(not shifts_in_place<decltype(xstd::bit_span(std::declval<array const&>()))>);
        static_assert(not shifts_in_place<decltype(std::declval<span&>().subspan(1UZ, 8UZ))>);
        static_assert(shifts_by_value<array> and shifts_by_value<xstd::bit_vector> and not shifts_by_value<span>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
