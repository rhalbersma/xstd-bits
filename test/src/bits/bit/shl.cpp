//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/enums.hpp>                   // day
#include <xstd/bits/bit/shl.hpp>                // shl
#include <xstd/bits/bit_set/bit_enum_set.hpp>   // bit_enum_set
#include <xstd/bits/bit_set/bit_fixed_set.hpp>  // bit_fixed_set
#include <xstd/bits/bit_set/bit_inplace_set.hpp> // bit_inplace_set
#include <xstd/bits/bit_set/bit_set.hpp>        // bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>   // bit_set_view
#include <xstd/ints/cstdint/int128.hpp>         // int128, uint128
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_TEST_CONTEXT
#include <algorithm>                            // ranges::equal
#include <cstddef>                              // size_t
#include <cstdint>                              // int64_t, int8_t, uint8_t
#include <limits>                               // numeric_limits
#include <ranges>                               // iota
#include <set>                                  // set

namespace {

// The keys of x moved s positions up, those landing outside [0, width) dropped: what shl answers.
template<class X>
[[nodiscard]] auto moved(X const& x, long long s, std::size_t width)
        -> std::set<std::size_t>
{
        auto nrv = std::set<std::size_t>();
        for (auto const k : x) {
                auto const p = static_cast<long long>(k) + s;
                if (0 <= p and p < static_cast<long long>(width)) {
                        nrv.insert(static_cast<std::size_t>(p));
                }
        }
        return nrv;
}

// Whether shl takes X by value.
template<class X>
concept shifts = requires (X x) { xstd::shl(x, 1); };

} // namespace

BOOST_AUTO_TEST_SUITE(Shl)

// Every count in the domain, past the width and negative included, at one block and at two.
BOOST_AUTO_TEST_CASE(AStaticWidthShiftsByEveryCountAndDropsWhatLeavesIt)
{
        auto const one = xstd::bit_fixed_set<64>({0UZ, 1UZ, 31UZ, 62UZ, 63UZ});
        auto const two = xstd::bit_fixed_set<100>({0UZ, 1UZ, 63UZ, 64UZ, 98UZ, 99UZ});
        for (auto const s : std::views::iota(-130LL, 131LL)) {
                BOOST_TEST_CONTEXT("count " << s)
                {
                        BOOST_CHECK(std::ranges::equal(xstd::shl(one, s), moved(one, s, 64UZ)));
                        BOOST_CHECK(std::ranges::equal(xstd::shl(two, s), moved(two, s, 100UZ)));
                }
        }
}

// The count's type changes nothing but which counts it can hold: the extremes of each empty the set.
BOOST_AUTO_TEST_CASE(EveryIntegerTypeIsACount)
{
        auto const x = xstd::bit_fixed_set<64>({3UZ, 40UZ});
        BOOST_CHECK(std::ranges::equal(xstd::shl(x, std::int8_t{-3}), std::set<std::size_t>{0UZ, 37UZ}));
        BOOST_CHECK(std::ranges::equal(xstd::shl(x, std::uint8_t{20}), std::set<std::size_t>{23UZ, 60UZ}));
        BOOST_CHECK(xstd::shl(x, std::numeric_limits<std::int64_t>::min()).empty());
        BOOST_CHECK(xstd::shl(x, std::numeric_limits<std::size_t>::max()).empty());
        BOOST_CHECK(xstd::shl(x, std::numeric_limits<xstd::uint128>::max()).empty());
        BOOST_CHECK(std::ranges::equal(xstd::shl(x, xstd::int128{1}), std::set<std::size_t>{4UZ, 41UZ}));
}

// A run-time width grows towards max_size() as its <<= does, and an inplace capacity drops what passes it.
BOOST_AUTO_TEST_CASE(EveryOwnerShiftsAsItsOwnOperatorDoes)
{
        auto const grown = xstd::shl(xstd::bit_set({1UZ, 5UZ}), 100);
        BOOST_CHECK(std::ranges::equal(grown, std::set<std::size_t>{101UZ, 105UZ}));
        BOOST_CHECK(xstd::shl(grown, std::numeric_limits<std::size_t>::max()).empty());

        auto const held = xstd::bit_inplace_set<8>({1UZ, 5UZ});
        BOOST_CHECK(std::ranges::equal(xstd::shl(held, 3), std::set<std::size_t>{4UZ}));

        using test::set::day;
        auto const days = xstd::bit_enum_set<day>{day::mon, day::thu};
        BOOST_CHECK(std::ranges::equal(xstd::shl(days, 1), xstd::bit_enum_set<day>{day::tue, day::fri}));
        BOOST_CHECK(xstd::shl(days, 5).empty());
}

// The total forms throw nothing where the set's own operators throw nothing.
BOOST_AUTO_TEST_CASE(AStaticWidthShiftThrowsNothing)
{
        auto const x = xstd::bit_fixed_set<64>();
        static_assert(noexcept(xstd::shl(x, 1)) and noexcept(xstd::shl(x, -1)) and noexcept(xstd::shl(x, 64)));
}

// A view refers to keys it does not own, so it is no set to shift by value: the copy would write through.
BOOST_AUTO_TEST_CASE(AViewIsNoSetToShift)
{
        auto x = xstd::bit_fixed_set<64>();
        auto v = xstd::bit_set_view(x);
        static_assert(not shifts<decltype(v)>);
        static_assert(shifts<decltype(x)>);
}

BOOST_AUTO_TEST_SUITE_END()
