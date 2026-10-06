//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/sequence/concepts.hpp>               // bit_sequence
#include <test/sequence/rotation.hpp>               // permutation_sweep
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector, bit_small_vector
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                 // same_as
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint8_t
#include <ranges>                                   // iota, random_access_range

BOOST_AUTO_TEST_SUITE(ExtBoostBitSmallVector)

namespace {

inline constexpr auto N = 256UZ;
using SmallVector       = xstd::bit_small_vector<N>;

} // namespace

// The short name is the general one at its defaults, and the allocator defaulted to is the storage's own.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<SmallVector, xstd::basic_bit_small_vector<std::size_t, N, boost::container::new_allocator<std::size_t>>>);
        static_assert(test::sequence::bit_sequence<SmallVector>);
        static_assert(std::ranges::random_access_range<SmallVector>);
        BOOST_CHECK(true);
}

// P3103R2's three inline and spilled: every width through three blocks, and whole and partial ones past the inline two.
BOOST_AUTO_TEST_CASE(ItRotatesAndReversesAsTheAlgorithmsDo)
{
        using V            = xstd::basic_bit_small_vector<std::uint8_t, 16>;
        auto disagreements = 0;
        for (auto const n : std::views::iota(0UZ, 18UZ)) {
                disagreements += test::sequence::permutation_sweep(V(n));
        }
        for (auto const n : {64UZ, 70UZ}) {
                disagreements += test::sequence::permutation_sweep(V(n));
        }
        BOOST_CHECK_EQUAL(disagreements, 0);
}

BOOST_AUTO_TEST_SUITE_END()
