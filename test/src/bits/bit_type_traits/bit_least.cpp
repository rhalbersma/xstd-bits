//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                 // bit_array
#include <xstd/bits/bit_set.hpp>                   // bit_set
#include <xstd/bits/bit_set_view.hpp>              // bit_set_view
#include <xstd/bits/bit_type_traits/bit_least.hpp> // bit_least, least_block_t
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                   // array
#include <bitset>                                  // bitset
#include <cstdint>                                 // uint16_t, uint32_t, uint64_t, uint8_t
#include <span>                                    // span
#include <type_traits>                             // is_same_v

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitLeast)

namespace {

template<class W>
concept least_rebinds = requires { typename xstd::bit_least<W>; };

} // namespace

// The narrowest fixed-width block holding N bits, the widest taking every N above it in several blocks.
BOOST_AUTO_TEST_CASE(TheLeastBlockIsTheNarrowestHoldingTheWidth)
{
        static_assert(std::is_same_v<xstd::least_block_t<0>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::least_block_t<8>, std::uint8_t>);
        static_assert(std::is_same_v<xstd::least_block_t<9>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::least_block_t<16>, std::uint16_t>);
        static_assert(std::is_same_v<xstd::least_block_t<17>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::least_block_t<32>, std::uint32_t>);
        static_assert(std::is_same_v<xstd::least_block_t<33>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<64>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<65>, std::uint64_t>);
        static_assert(std::is_same_v<xstd::least_block_t<1000>, std::uint64_t>);
        BOOST_CHECK(true);
}

// The transformation rewrites a width in the type, so a run-time width, a view or a bare block has none to give.
BOOST_AUTO_TEST_CASE(OnlyAFixedWidthOwnerIsTransformed)
{
        static_assert(least_rebinds<xstd::bit_array<9>>);
        static_assert(not least_rebinds<xstd::bit_set>);
        static_assert(not least_rebinds<xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not least_rebinds<std::bitset<9>> and not least_rebinds<std::uint64_t> and not least_rebinds<std::array<std::uint8_t, 2>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
