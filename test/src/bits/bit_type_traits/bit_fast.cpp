//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                // bit_array
#include <xstd/bits/bit_set.hpp>                  // bit_set
#include <xstd/bits/bit_set_view.hpp>             // bit_set_view
#include <xstd/bits/bit_type_traits/bit_fast.hpp> // bit_fast, fast_block_t
#include <boost/test/unit_test.hpp>               // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                  // array
#include <bitset>                                 // bitset
#include <cstdint>                                // uint32_t, uint64_t, uint8_t, uint_fast16_t, uint_fast32_t, uint_fast64_t, uint_fast8_t
#include <span>                                   // span
#include <type_traits>                            // is_same_v

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitFast)

namespace {

template<class W>
concept fast_rebinds = requires { typename xstd::bit_fast<W>; };

} // namespace

// The fastest block of at least N bits is <cstdint>'s, whose width the platform chooses, so only its name is fixed.
BOOST_AUTO_TEST_CASE(TheFastBlockIsTheFastestOfAtLeastTheWidth)
{
        static_assert(std::is_same_v<xstd::fast_block_t<0>, std::uint_fast8_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<8>, std::uint_fast8_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<9>, std::uint_fast16_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<16>, std::uint_fast16_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<17>, std::uint_fast32_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<32>, std::uint_fast32_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<33>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<64>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<65>, std::uint_fast64_t>);
        static_assert(std::is_same_v<xstd::fast_block_t<1000>, std::uint_fast64_t>);
        BOOST_CHECK(true);
}

// The transformation rewrites a width in the type, so a run-time width, a view or a bare block has none to give.
BOOST_AUTO_TEST_CASE(OnlyAFixedWidthOwnerIsTransformed)
{
        static_assert(fast_rebinds<xstd::bit_array<9>>);
        static_assert(not fast_rebinds<xstd::bit_set>);
        static_assert(not fast_rebinds<xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not fast_rebinds<std::bitset<9>> and not fast_rebinds<std::uint64_t> and not fast_rebinds<std::array<std::uint8_t, 2>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
