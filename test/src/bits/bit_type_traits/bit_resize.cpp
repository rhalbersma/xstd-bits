//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_fixed_set.hpp>              // bit_fixed_set
#include <xstd/bits/bit_set.hpp>                    // bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_type_traits/bit_resize.hpp> // bit_resize
#include <xstd/bits/bit_vector.hpp>                 // bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector
#include <boost/container/new_allocator.hpp>        // new_allocator
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                                    // array
#include <bitset>                                   // bitset
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint32_t, uint64_t, uint8_t
#include <span>                                     // span
#include <type_traits>                              // is_same_v

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitResize)

namespace {

template<std::size_t N, class W>
concept resizes = requires { typename xstd::bit_resize<N, W>; };

} // namespace

// The width in the type is replaced, the block and every other argument kept, an allocator included.
BOOST_AUTO_TEST_CASE(TheWidthIsReplacedAndTheRestKept)
{
        static_assert(std::is_same_v<xstd::bit_resize<16, xstd::basic_bit_array<std::uint8_t, 9>>, xstd::basic_bit_array<std::uint8_t, 16>>);
        static_assert(std::is_same_v<xstd::bit_resize<3, xstd::basic_bit_bounded_vector<std::uint32_t, 9>>, xstd::basic_bit_bounded_vector<std::uint32_t, 3>>);
        static_assert(std::is_same_v<xstd::bit_resize<64, xstd::bit_fixed_set<9>>, xstd::bit_fixed_set<64>>);
        static_assert(std::is_same_v<xstd::bit_resize<0, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 9>>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 0>>);
        static_assert(std::is_same_v<xstd::bit_resize<128, xstd::basic_bit_small_vector<std::uint64_t, 9>>, xstd::basic_bit_small_vector<std::uint64_t, 128, boost::container::new_allocator<std::uint64_t>>>);
        static_assert(std::is_same_v<xstd::bit_resize<128, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 9>>, xstd::basic_bit_small_set<std::size_t, std::uint64_t, 128>>);
        static_assert(std::is_same_v<xstd::bit_resize<9, xstd::basic_bit_array<std::uint8_t, 9>>, xstd::basic_bit_array<std::uint8_t, 9>>);
        BOOST_CHECK(true);
}

// A run-time width, a view, a standard bit container, a bare block and a range of blocks have no width in the type.
BOOST_AUTO_TEST_CASE(OnlyAnOwnerWithAWidthInItsTypeIsResized)
{
        static_assert(not resizes<16, xstd::bit_vector> and not resizes<16, xstd::bit_set>);
        static_assert(not resizes<16, xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not resizes<16, std::bitset<9>> and not resizes<16, std::uint64_t> and not resizes<16, std::array<std::uint8_t, 2>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
