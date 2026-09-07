//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SEQUENCE_ORDERING_HPP
#define TEST_SEQUENCE_ORDERING_HPP

#include <boost/test/unit_test.hpp>           // BOOST_CHECK_EQUAL
#include <test/bitset/factory.hpp>            // make_bitset
#include <xstd/bits/bit_span.hpp> // bit_span
#include <algorithm>                          // equal, lexicographical_compare, lexicographical_compare_three_way
#include <compare>                            // strong_ordering
#include <cstddef>                            // size_t
#include <vector>                             // vector

namespace test::sequence {

// What the sequence reading must order like, against the container defining the relation; the view itself neither compares nor orders, following span, so the question goes through its iterators.
template<class Bits>
auto ordering_agrees_with_vector_bool(std::size_t universe = 4) -> void
{
        auto const bound = 1UZ << universe;
        for (auto i = 0UZ; i < bound; ++i) {
                for (auto j = 0UZ; j < bound; ++j) {
                        auto x = test::bitset::make_bitset<Bits>(universe);
                        auto y = test::bitset::make_bitset<Bits>(universe);

                        // Written through the view; named rather than inlined, because CTAD followed by [k] parses as an array declaration.
                        auto xw = xstd::bit_span(x);
                        auto yw = xstd::bit_span(y);
                        for (auto k = 0UZ; k < universe; ++k) {
                                xw[k] = (i >> k & 1UZ) != 0UZ;
                                yw[k] = (j >> k & 1UZ) != 0UZ;
                        }

                        auto const xv = xstd::bit_span(x);
                        auto const yv = xstd::bit_span(y);

                        // The reference holds the same bools at the same positions, over the whole width the view reports.
                        auto vx = std::vector<bool>(xv.size());
                        auto vy = std::vector<bool>(yv.size());
                        for (auto k = 0UZ; k < xv.size(); ++k) { vx[k] = static_cast<bool>(xv[k]); }
                        for (auto k = 0UZ; k < yv.size(); ++k) { vy[k] = static_cast<bool>(yv[k]); }

                        BOOST_CHECK_EQUAL(std::ranges::equal(xv, yv), vx == vy);
                        auto const order = std::lexicographical_compare_three_way(xv.begin(), xv.end(), yv.begin(), yv.end());
                        BOOST_CHECK_EQUAL(order < 0, std::lexicographical_compare(vx.begin(), vx.end(), vy.begin(), vy.end()));
                        BOOST_CHECK_EQUAL(order > 0, std::lexicographical_compare(vy.begin(), vy.end(), vx.begin(), vx.end()));
                }
        }
}

} // namespace test::sequence

#endif // TEST_SEQUENCE_ORDERING_HPP
