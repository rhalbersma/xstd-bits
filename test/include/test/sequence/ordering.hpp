//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_ORDERING_HPP
#define TEST_SEQUENCE_ORDERING_HPP

#include <test/bitset/factory.hpp>  // make_bitset
#include <xstd/bits/bit_span.hpp>   // bit_span
#include <boost/test/unit_test.hpp> // BOOST_CHECK_EQUAL
#include <algorithm>                // equal, lexicographical_compare, lexicographical_compare_three_way
#include <compare>                  // is_gt, is_lt, strong_ordering
#include <cstddef>                  // size_t
#include <ranges>                   // iota
#include <vector>                   // vector

namespace test::sequence {

// What the sequence reading must order like; the view neither compares nor orders, so it goes by iterators.
template<class Bits>
auto ordering_agrees_with_vector_bool(std::size_t universe = 4)
        -> void
{
        auto const bound = 1UZ << universe;
        for (auto const i : std::views::iota(0UZ, bound)) {
                for (auto const j : std::views::iota(0UZ, bound)) {
                        auto x = test::bitset::make_bitset<Bits>(universe);
                        auto y = test::bitset::make_bitset<Bits>(universe);

                        // Written through the view; named, CTAD followed by [k] parsing as an array declaration.
                        auto xw = xstd::bit_span(x);
                        auto yw = xstd::bit_span(y);
                        for (auto const k : std::views::iota(0UZ, universe)) {
                                xw[k] = (i >> k & 1UZ) != 0UZ;
                                yw[k] = (j >> k & 1UZ) != 0UZ;
                        }

                        auto const xv = xstd::bit_span(x);
                        auto const yv = xstd::bit_span(y);

                        // The reference holds the same bools at the same positions, over the whole width.
                        auto vx = std::vector<bool>(xv.size());
                        auto vy = std::vector<bool>(yv.size());
                        for (auto const k : std::views::iota(0UZ, xv.size())) {
                                vx[k] = static_cast<bool>(xv[k]);
                        }
                        for (auto const k : std::views::iota(0UZ, yv.size())) {
                                vy[k] = static_cast<bool>(yv[k]);
                        }

                        BOOST_CHECK_EQUAL(std::ranges::equal(xv, yv), vx == vy);
                        auto const order = std::lexicographical_compare_three_way(xv.begin(), xv.end(), yv.begin(), yv.end());
                        BOOST_CHECK_EQUAL(std::is_lt(order), std::ranges::lexicographical_compare(vx, vy));
                        BOOST_CHECK_EQUAL(std::is_gt(order), std::ranges::lexicographical_compare(vy, vx));
                }
        }
}

} // namespace test::sequence

#endif // TEST_SEQUENCE_ORDERING_HPP
