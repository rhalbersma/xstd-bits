//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/set/operations.hpp>                          // disagreements
#include <xstd/bits/bit_algorithm/bit_set_intersection.hpp> // bit_set_intersection
#include <xstd/bits/bit_array.hpp>                          // bit_array
#include <xstd/bits/bit_set/bit_fixed_set.hpp>              // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_set/bit_flag_set.hpp>               // bit_flag_set
#include <xstd/bits/bit_set/bit_key_mapping.hpp>            // bit_key_mapping
#include <xstd/bits/bit_set/bit_set.hpp>                    // bit_set
#include <xstd/bits/bit_set/bit_set_view.hpp>               // bit_set_view
#include <boost/test/unit_test.hpp>                         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <range/v3/view/set_algorithm.hpp>                  // set_intersection
#include <concepts>                                         // same_as
#include <cstddef>                                          // size_t
#include <cstdint>                                          // uint64_t, uint8_t
#include <filesystem>                                       // perms
#include <functional>                                       // bit_and, greater, less
#include <ranges>                                           // to
#include <set>                                              // set
#include <utility>                                          // declval

BOOST_AUTO_TEST_SUITE(BitSetIntersection)

namespace {

template<class S1, class S2>
concept askable = requires (S1 const& s1, S2 const& s2) { xstd::bit_set_intersection(s1, s2); };

constexpr auto algorithm = [](auto const& s1, auto const& s2) { return xstd::bit_set_intersection(s1, s2); };
constexpr auto view      = [](auto const& s1, auto const& s2, auto comp) { return ::ranges::views::set_intersection(s1, s2, comp); };
constexpr auto op        = std::bit_and<>();

} // namespace

// Every pair of subsets of six keys, in one block, across a block boundary, in descending order and at run-time width.
BOOST_AUTO_TEST_CASE(AgreesWithRangeV3AndWithItsOperator)
{
        BOOST_CHECK_EQUAL(test::set::operations::disagreements<xstd::bit_fixed_set<6>>(6UZ, algorithm, view, op), 0);
        BOOST_CHECK_EQUAL((test::set::operations::disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 10>>(6UZ, algorithm, view, op)), 0);
        BOOST_CHECK_EQUAL((test::set::operations::disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 6, xstd::bit_key_mapping<std::size_t>, std::greater<>>>(6UZ, algorithm, view, op)), 0);
        BOOST_CHECK_EQUAL(test::set::operations::disagreements<xstd::bit_set>(6UZ, algorithm, view, op), 0);
}

// Run-time widths that differ: the result is the set of keys, whatever the widths that held them.
BOOST_AUTO_TEST_CASE(TwoRunTimeWidthsCombineTheirKeys)
{
        auto const narrow = xstd::bit_set({1UZ, 3UZ});
        auto const wide   = xstd::bit_set({3UZ, 200UZ});
        auto const keys   = [](xstd::bit_set const& s) -> std::set<std::size_t> { return {s.begin(), s.end()}; };
        BOOST_CHECK(keys(xstd::bit_set_intersection(narrow, wide)) == (view(keys(narrow), keys(wide), std::less<>()) | std::ranges::to<std::set<std::size_t>>()));
        BOOST_CHECK(keys(xstd::bit_set_intersection(wide, narrow)) == (view(keys(wide), keys(narrow), std::less<>()) | std::ranges::to<std::set<std::size_t>>()));
}

// The result has the first argument's type, a flag converts as the second; views, sequences and mixed sets are refused.
BOOST_AUTO_TEST_CASE(WhatItTakesAndGives)
{
        static_assert(xstd::bit_set_intersection(xstd::bit_fixed_set<8>{1, 2}, xstd::bit_fixed_set<8>{2, 3}) == xstd::bit_fixed_set<8>{2});
        static_assert(std::same_as<decltype(xstd::bit_set_intersection(xstd::bit_set(), xstd::bit_set())), xstd::bit_set>);
        using perms = xstd::bit_flag_set<std::filesystem::perms, 16>;
        BOOST_CHECK(xstd::bit_set_intersection(perms(std::filesystem::perms::owner_all), std::filesystem::perms::owner_read) == perms(std::filesystem::perms::owner_read));
        using view_type = decltype(xstd::bit_set_view(std::declval<std::uint64_t&>()));
        static_assert(not askable<view_type, view_type>);
        static_assert(askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<8>> and not askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<9>>);
        static_assert(not askable<xstd::bit_array<8>, xstd::bit_array<8>> and not askable<std::set<int>, std::set<int>>);
}

BOOST_AUTO_TEST_SUITE_END()
