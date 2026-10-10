//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/algorithm/bit_disjoint.hpp> // bit_disjoint
#include <xstd/bits/bit_array.hpp>              // bit_array
#include <xstd/bits/bit_fixed_set.hpp>          // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_set.hpp>           // bit_flag_set
#include <xstd/bits/bit_key_mapping.hpp>        // bit_key_mapping
#include <xstd/bits/bit_set.hpp>                // bit_set
#include <xstd/bits/bit_set_view.hpp>           // bit_set_view
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                            // includes, set_intersection
#include <cstddef>                              // size_t
#include <cstdint>                              // uint64_t, uint8_t
#include <filesystem>                           // perms
#include <functional>                           // greater
#include <iterator>                             // back_inserter
#include <ranges>                               // iota
#include <set>                                  // set
#include <vector>                               // vector

BOOST_AUTO_TEST_SUITE(BitDisjoint)

namespace {

template<class S1, class S2>
concept askable = requires (S1 const& s1, S2 const& s2) { xstd::bit_disjoint(s1, s2); };

// The model: whether std::ranges::set_intersection over the keys in the order the sets share finds none.
template<class Set>
[[nodiscard]] auto model(Set const& s1, Set const& s2)
        -> bool
{
        auto common = std::vector<typename Set::key_type>();
        std::ranges::set_intersection(s1, s2, std::back_inserter(common), s1.key_comp());
        return common.empty();
}

// Every pair of subsets of the first n keys, as masks, in a set of N positions, against the model on std::set.
template<class S>
[[nodiscard]] auto disagreements(std::size_t n)
        -> int
{
        auto wrong = 0;
        for (auto const x : std::views::iota(0UZ, 1UZ << n)) {
                for (auto const y : std::views::iota(0UZ, 1UZ << n)) {
                        auto s1 = S();
                        auto s2 = S();
                        for (auto const i : std::views::iota(0UZ, n)) {
                                if (((x >> i) & 1UZ) != 0UZ) {
                                        s1.insert(i);
                                }
                                if (((y >> i) & 1UZ) != 0UZ) {
                                        s2.insert(i);
                                }
                        }
                        wrong += static_cast<int>(xstd::bit_disjoint(s1, s2) != model(std::set(s1.begin(), s1.end(), s1.key_comp()), std::set(s2.begin(), s2.end(), s1.key_comp())));
                }
        }
        return wrong;
}

} // namespace

// Every pair of subsets of six keys, in one block, across a block boundary, in descending order and at run-time width.
BOOST_AUTO_TEST_CASE(AgreesWithTheModelOnStdSet)
{
        BOOST_CHECK_EQUAL(disagreements<xstd::bit_fixed_set<6>>(6UZ), 0);
        BOOST_CHECK_EQUAL((disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 10>>(6UZ)), 0);
        BOOST_CHECK_EQUAL((disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 6, xstd::bit_key_mapping<std::size_t>, std::greater<std::size_t>>>(6UZ)), 0);
        BOOST_CHECK_EQUAL(disagreements<xstd::bit_set>(6UZ), 0);
}

// Run-time widths that differ: the shorter's missing blocks hold no key.
BOOST_AUTO_TEST_CASE(TwoRunTimeWidthsCompareTheirKeys)
{
        auto const narrow   = xstd::bit_set({1UZ, 3UZ});
        auto const wide     = xstd::bit_set({1UZ, 3UZ, 200UZ});
        auto const far      = xstd::bit_set({200UZ});
        auto const model_of = [](xstd::bit_set const& s) -> std::set<std::size_t> { return {s.begin(), s.end()}; };
        for (auto const* a : {&narrow, &wide, &far}) {
                for (auto const* b : {&narrow, &wide, &far}) {
                        BOOST_CHECK_EQUAL(xstd::bit_disjoint(*a, *b), model(model_of(*a), model_of(*b)));
                }
        }
}

// A view asks what it views; a flag converts to its flag set as the second argument; sequences and mixed sets are no sets.
BOOST_AUTO_TEST_CASE(WhatItAsks)
{
        auto blocks     = std::uint64_t{0b1010};
        auto const view = xstd::bit_set_view(blocks);
        auto const keys = std::set<std::size_t>({1UZ, 3UZ});
        BOOST_CHECK_EQUAL(xstd::bit_disjoint(view, view), model(keys, keys));
        using perms = xstd::bit_flag_set<std::filesystem::perms, 16>;
        BOOST_CHECK(not xstd::bit_disjoint(perms(std::filesystem::perms::owner_all), std::filesystem::perms::owner_read));
        static_assert(askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<8>> and not askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<9>>);
        static_assert(not askable<xstd::bit_array<8>, xstd::bit_array<8>> and not askable<std::set<int>, std::set<int>>);
}

BOOST_AUTO_TEST_SUITE_END()
