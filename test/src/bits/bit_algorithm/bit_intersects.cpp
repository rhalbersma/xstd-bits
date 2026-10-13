//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_algorithm/bit_disjoint.hpp>   // bit_disjoint
#include <xstd/bits/bit_algorithm/bit_intersects.hpp> // bit_intersects
#include <xstd/bits/bit_array.hpp>                    // bit_array
#include <xstd/bits/bit_fixed_set.hpp>                // basic_bit_fixed_set, bit_fixed_set
#include <xstd/bits/bit_flag_set.hpp>                 // bit_flag_set
#include <xstd/bits/bit_key_mapping.hpp>              // bit_key_mapping
#include <xstd/bits/bit_set.hpp>                      // bit_set
#include <xstd/bits/bit_set_view.hpp>                 // bit_set_view
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                  // includes, set_intersection
#include <array>                                      // array
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint64_t, uint8_t
#include <filesystem>                                 // perms
#include <functional>                                 // greater
#include <iterator>                                   // back_inserter
#include <ranges>                                     // iota
#include <set>                                        // set
#include <vector>                                     // vector

BOOST_AUTO_TEST_SUITE(BitIntersects)

namespace {

template<class S1, class S2>
concept askable = requires (S1 const& s1, S2 const& s2) { xstd::bit_intersects(s1, s2); };

// The model: whether std::ranges::set_intersection over the keys in the order the sets share finds one.
template<class Set>
[[nodiscard]] auto model(Set const& s1, Set const& s2)
        -> bool
{
        auto common = std::vector<typename Set::key_type>();
        std::ranges::set_intersection(s1, s2, std::back_inserter(common), s1.key_comp());
        return not common.empty();
}

// Every pair of subsets of the given keys, as masks, against the model on std::set.
template<class S, std::size_t K>
[[nodiscard]] auto disagreements(std::array<std::size_t, K> const& keys)
        -> int
{
        auto wrong = 0;
        for (auto const x : std::views::iota(0UZ, 1UZ << K)) {
                for (auto const y : std::views::iota(0UZ, 1UZ << K)) {
                        auto s1 = S();
                        auto s2 = S();
                        for (auto const i : std::views::iota(0UZ, K)) {
                                if (((x >> i) & 1UZ) != 0UZ) {
                                        s1.insert(keys[i]);
                                }
                                if (((y >> i) & 1UZ) != 0UZ) {
                                        s2.insert(keys[i]);
                                }
                        }
                        wrong += static_cast<int>(xstd::bit_intersects(s1, s2) != model(std::set(s1.begin(), s1.end(), s1.key_comp()), std::set(s2.begin(), s2.end(), s1.key_comp())));
                }
        }
        return wrong;
}

} // namespace

// Every pair of subsets of six keys: in one block, across two and three, descending, and at run-time width.
BOOST_AUTO_TEST_CASE(AgreesWithTheModelOnStdSet)
{
        constexpr auto first_six = std::array{0UZ, 1UZ, 2UZ, 3UZ, 4UZ, 5UZ};
        BOOST_CHECK_EQUAL(disagreements<xstd::bit_fixed_set<6>>(first_six), 0);
        BOOST_CHECK_EQUAL((disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 10>>(std::array{0UZ, 2UZ, 4UZ, 7UZ, 8UZ, 9UZ})), 0);
        BOOST_CHECK_EQUAL((disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 20>>(std::array{0UZ, 5UZ, 8UZ, 12UZ, 16UZ, 19UZ})), 0);
        BOOST_CHECK_EQUAL((disagreements<xstd::basic_bit_fixed_set<std::size_t, std::uint64_t, 6, xstd::bit_key_mapping<std::size_t>, std::greater<>>>(first_six)), 0);
        BOOST_CHECK_EQUAL(disagreements<xstd::bit_set>(first_six), 0);
}

// Width zero holds no key, so no key meets another.
BOOST_AUTO_TEST_CASE(WidthZero)
{
        auto const none = xstd::bit_fixed_set<0>();
        BOOST_CHECK(not xstd::bit_intersects(none, none));
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
                        BOOST_CHECK_EQUAL(xstd::bit_intersects(*a, *b), model(model_of(*a), model_of(*b)));
                        BOOST_CHECK_EQUAL(xstd::bit_intersects(*a, *b), not xstd::bit_disjoint(*a, *b));
                }
        }
}

// A view asks what it views; a flag converts to its flag set; sequences and mixed sets are no sets.
BOOST_AUTO_TEST_CASE(WhatItAsks)
{
        auto blocks     = std::uint64_t{0b1010};
        auto const view = xstd::bit_set_view(blocks);
        auto const keys = std::set<std::size_t>({1UZ, 3UZ});
        BOOST_CHECK_EQUAL(xstd::bit_intersects(view, view), model(keys, keys));
        using perms = xstd::bit_flag_set<std::filesystem::perms, 16>;
        BOOST_CHECK(xstd::bit_intersects(perms(std::filesystem::perms::owner_all), std::filesystem::perms::owner_read));
        BOOST_CHECK(not xstd::bit_intersects(perms(std::filesystem::perms::owner_all), std::filesystem::perms::others_read));
        static_assert(askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<8>> and not askable<xstd::bit_fixed_set<8>, xstd::bit_fixed_set<9>>);
        static_assert(not askable<xstd::bit_array<8>, xstd::bit_array<8>> and not askable<std::set<int>, std::set<int>>);
}

BOOST_AUTO_TEST_SUITE_END()
