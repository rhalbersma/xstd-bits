//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_SET_ORDERING_HPP
#define TEST_SET_ORDERING_HPP

#include <test/bitset/factory.hpp>    // make_bitset
#include <xstd/bits/bit_set_view.hpp> // bit_set_view
#include <boost/test/unit_test.hpp>   // BOOST_CHECK_EQUAL
#include <algorithm>                  // lexicographical_compare
#include <compare>                    // is_gt, is_lt, strong_ordering
#include <cstddef>                    // size_t
#include <cstdint>                    // uint64_t
#include <set>                        // set

namespace test::set {

// What the set reading must order like, against std::set: set_compare's default trusts the viewed type's <=>, and dynamic_bitset's is wrong. [design.md#counted-not-asserted] [design.md#the-ordering-primitive]
template<class Bits>
auto ordering_agrees_with_std_set(std::size_t universe = 4)
        -> void
{
        auto const bound = 1UZ << universe;
        auto equality_disagreements = 0UZ;
        auto less_disagreements     = 0UZ;
        auto greater_disagreements  = 0UZ;

        for (auto i = 0UZ; i < bound; ++i) {
                for (auto j = 0UZ; j < bound; ++j) {
                        auto x = test::bitset::make_bitset<Bits>(universe);
                        auto y = test::bitset::make_bitset<Bits>(universe);
                        auto kx = std::set<std::size_t>();
                        auto ky = std::set<std::size_t>();

                        // Written through the view, in the set vocabulary, which is the interface under test rather than the bitset's own; named, because clang 23's lifetime analysis crashes on a deducing-this member called on a prvalue.
                        auto const xw = xstd::bit_set_view(x);
                        auto const yw = xstd::bit_set_view(y);
                        for (auto k = 0UZ; k < universe; ++k) {
                                if (i >> k & 1UZ) { xw.insert(k); kx.insert(k); }
                                if (j >> k & 1UZ) { yw.insert(k); ky.insert(k); }
                        }

                        auto const xv = xstd::bit_set_view(x);
                        auto const yv = xstd::bit_set_view(y);

                        equality_disagreements += static_cast<std::size_t>((xv == yv) != (kx == ky));
                        less_disagreements     += static_cast<std::size_t>(
                                std::is_lt(xv <=> yv) != std::ranges::lexicographical_compare(kx, ky));
                        greater_disagreements  += static_cast<std::size_t>(
                                std::is_gt(xv <=> yv) != std::ranges::lexicographical_compare(ky, kx));
                }
        }

        BOOST_CHECK_EQUAL(equality_disagreements, 0UZ);
        BOOST_CHECK_EQUAL(less_disagreements,     0UZ);
        BOOST_CHECK_EQUAL(greater_disagreements,  0UZ);
}

// Three blocks and up, where an exhaustive sweep is no longer affordable: 2^18 squared is not a test. [design.md#the-ordering-primitive]
template<class Bits>
auto ordering_agrees_with_std_set_sampled(std::size_t universe, std::size_t trials)
        -> void
{
        auto equality_disagreements = 0UZ;
        auto less_disagreements     = 0UZ;
        auto greater_disagreements  = 0UZ;
        // Fixed width, not ULL: the sequence a fixed seed reproduces should not depend on how wide the platform makes unsigned long long.
        auto lcg = std::uint64_t{0x9E3779B97F4A7C15};
        auto const next = [&lcg] -> std::uint64_t { lcg = (lcg * 6364136223846793005ULL) + 1442695040888963407ULL; return lcg >> 11U; };

        for (auto t = 0UZ; t < trials; ++t) {
                auto const i = next();
                auto const j = next();

                auto x = test::bitset::make_bitset<Bits>(universe);
                auto y = test::bitset::make_bitset<Bits>(universe);
                auto kx = std::set<std::size_t>();
                auto ky = std::set<std::size_t>();

                auto const xw = xstd::bit_set_view(x);
                auto const yw = xstd::bit_set_view(y);
                for (auto k = 0UZ; k < universe; ++k) {
                        if (i >> k & 1UZ) { xw.insert(k); kx.insert(k); }
                        if (j >> k & 1UZ) { yw.insert(k); ky.insert(k); }
                }

                auto const xv = xstd::bit_set_view(x);
                auto const yv = xstd::bit_set_view(y);

                equality_disagreements += static_cast<std::size_t>((xv == yv) != (kx == ky));
                less_disagreements     += static_cast<std::size_t>(
                        std::is_lt(xv <=> yv) != std::ranges::lexicographical_compare(kx, ky));
                greater_disagreements  += static_cast<std::size_t>(
                        std::is_gt(xv <=> yv) != std::ranges::lexicographical_compare(ky, kx));
        }

        BOOST_CHECK_EQUAL(equality_disagreements, 0UZ);
        BOOST_CHECK_EQUAL(less_disagreements,     0UZ);
        BOOST_CHECK_EQUAL(greater_disagreements,  0UZ);
}

} // namespace test::set

#endif // TEST_SET_ORDERING_HPP
