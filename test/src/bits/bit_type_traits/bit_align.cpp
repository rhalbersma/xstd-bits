//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                 // basic_bit_array, bit_array
#include <xstd/bits/bit_set.hpp>                   // bit_set
#include <xstd/bits/bit_set_view.hpp>              // bit_set_view
#include <xstd/bits/bit_type_traits/bit_align.hpp> // bit_align
#include <boost/test/unit_test.hpp>                // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                   // array
#include <bitset>                                  // bitset
#include <cstdint>                                 // uint32_t, uint64_t, uint8_t
#include <span>                                    // span
#include <type_traits>                             // is_same_v

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitAlign)

namespace {

template<class W>
concept align_rebinds = requires { typename xstd::bit_align<W>; };

} // namespace

// The width rounds up to whole blocks, the block kept, and a width already whole or zero stays as it is.
BOOST_AUTO_TEST_CASE(TheWidthRoundsUpToWholeBlocks)
{
        static_assert(std::is_same_v<xstd::bit_align<xstd::basic_bit_array<std::uint8_t, 9>>, xstd::basic_bit_array<std::uint8_t, 16>>);
        static_assert(std::is_same_v<xstd::bit_align<xstd::basic_bit_array<std::uint8_t, 16>>, xstd::basic_bit_array<std::uint8_t, 16>>);
        static_assert(std::is_same_v<xstd::bit_align<xstd::basic_bit_array<std::uint8_t, 0>>, xstd::basic_bit_array<std::uint8_t, 0>>);
        BOOST_CHECK(true);
}

// The transformation rewrites a width in the type, so a run-time width, a view or a bare block has none to give.
BOOST_AUTO_TEST_CASE(OnlyAFixedWidthOwnerIsTransformed)
{
        static_assert(align_rebinds<xstd::bit_array<9>>);
        static_assert(not align_rebinds<xstd::bit_set>);
        static_assert(not align_rebinds<xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not align_rebinds<std::bitset<9>> and not align_rebinds<std::uint64_t> and not align_rebinds<std::array<std::uint8_t, 2>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
