//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/ext/boost/small_bitset.hpp> // basic_small_bitset, small_bitset
#include <boost/test/unit_test.hpp>             // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <bitset>                               // bitset
#include <concepts>                             // regular, same_as, totally_ordered
#include <cstddef>                              // size_t

BOOST_AUTO_TEST_SUITE(ExtBoostSmallBitset)

namespace {

inline constexpr auto N = 256UZ;
using SmallBitset = xstd::small_bitset<N>;

} // namespace

// The short name is the general one at its defaults, and the allocator defaulted to is the storage's own.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<SmallBitset, xstd::basic_small_bitset<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        static_assert(std::regular<SmallBitset> and std::totally_ordered<SmallBitset>);
        BOOST_CHECK(true);
}

// A width past the inline capacity, so the shift crosses the boundary the other columns do not have.
BOOST_AUTO_TEST_CASE(TheBitsetReadingAnswersStdBitset)
{
        auto oracle = std::bitset<300>();
        auto ours = SmallBitset(300);
        for (auto const i : {5UZ, 100UZ, 299UZ}) {
                oracle.set(i);
                ours.set(i);
        }
        BOOST_CHECK_EQUAL(ours.count(), oracle.count());
        BOOST_CHECK_EQUAL(ours.to_string(), oracle.to_string());

        ours <<= 7;
        oracle <<= 7;
        BOOST_CHECK_EQUAL(ours.to_string(), oracle.to_string());
}

BOOST_AUTO_TEST_SUITE_END()
