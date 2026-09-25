//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit/bit_cast.hpp>     // bit_cast, bit_castable
#include <xstd/bits/bit_array.hpp>        // bit_array
#include <xstd/bits/bit_set.hpp>          // bit_set
#include <xstd/bits/bit_set_view.hpp>     // bit_set_view
#include <xstd/bits/bit_span.hpp>         // bit_span
#include <xstd/bits/bit_static_set.hpp>   // bit_static_set
#include <xstd/bits/bitset.hpp>           // bitset
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                          // array
#include <bitset>                         // bitset
#include <cstdint>                        // uint8_t, uint32_t, uint64_t
#include <vector>                         // vector

BOOST_AUTO_TEST_SUITE(BitCast)

namespace {

template<class To, class From>
concept casts = requires (From const& from) { xstd::bit_cast<To>(from); };

} // namespace

// What has bit storage of a fixed width: our owners and full-width views, words and arrays of them, a std::bitset.
BOOST_AUTO_TEST_CASE(WhatHasBitStorageOfAFixedWidthIsCastable)
{
        static_assert(xstd::bit_castable<std::uint64_t> and xstd::bit_castable<std::array<std::uint8_t, 3>>);
        static_assert(xstd::bit_castable<xstd::bit_static_set<64>> and xstd::bit_castable<xstd::bit_array<20>> and xstd::bit_castable<xstd::bitset<64>>);
        static_assert(xstd::bit_castable<xstd::bit_set_view<std::uint64_t>> and xstd::bit_castable<xstd::bit_span<std::array<std::uint8_t, 3>>>);
        static_assert(xstd::bit_castable<std::bitset<64>>);
        static_assert(not xstd::bit_castable<xstd::bit_set> and not xstd::bit_castable<std::vector<std::uint32_t>>);
        static_assert(not xstd::bit_castable<int> and not xstd::bit_castable<std::vector<bool>>);
        BOOST_CHECK(true);
}

// Between any two readings, and to and from the words and a std::bitset: the blocks are copied, whatever each reads.
BOOST_AUTO_TEST_CASE(TheBlocksAreCopiedAcrossReadings)
{
        static_assert([] -> bool {
                auto const set = xstd::bit_static_set<64>{0, 5, 63};
                auto const word = xstd::bit_cast<std::uint64_t>(set);
                auto const seq = xstd::bit_cast<xstd::bit_array<64>>(set);
                auto const bits = xstd::bit_cast<xstd::bitset<64>>(seq);
                auto const legacy = xstd::bit_cast<std::bitset<64>>(bits);
                return word == ((1ULL << 63U) | (1ULL << 5U) | 1ULL) and seq[5] and bits.test(63) and legacy.count() == 3 and xstd::bit_cast<xstd::bit_static_set<64>>(legacy) == set;
        }());

        // A width that is no whole number of blocks round-trips through a std::bitset of the same width.
        auto const narrow = xstd::bit_array<20>(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x08});
        auto const legacy = xstd::bit_cast<std::bitset<20>>(narrow);
        BOOST_CHECK(legacy.test(0) and legacy.test(19));
        BOOST_CHECK(xstd::bit_cast<xstd::bit_array<20>>(legacy) == narrow);

        // Width zero has no bytes to copy, and still casts.
        BOOST_CHECK(xstd::bit_cast<std::bitset<0>>(xstd::bit_array<0>()).none());
}

// A view is read from as the words it spans, and never written into.
BOOST_AUTO_TEST_CASE(AViewIsCastFromAndNotInto)
{
        auto board = std::uint64_t{0b1010};
        BOOST_CHECK_EQUAL(xstd::bit_cast<std::uint64_t>(xstd::bit_set_view(board)), board);
        static_assert(casts<std::uint64_t, xstd::bit_set_view<std::uint64_t>>);
        static_assert(not casts<xstd::bit_set_view<std::uint64_t>, std::uint64_t>);
}

// The widths agree exactly, as std::bit_cast's sizes do: nothing is truncated and nothing is padded.
BOOST_AUTO_TEST_CASE(TheWidthsAgreeExactly)
{
        static_assert(casts<xstd::bit_array<64>, std::uint64_t> and casts<std::bitset<20>, xstd::bit_array<20>>);
        static_assert(not casts<xstd::bit_array<20>, std::uint32_t> and not casts<std::bitset<63>, std::uint64_t>);
        static_assert(not casts<xstd::bit_array<64>, xstd::bitset<65>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
