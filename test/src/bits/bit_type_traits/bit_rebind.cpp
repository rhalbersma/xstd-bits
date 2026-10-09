//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>                  // basic_bit_array
#include <xstd/bits/bit_bounded_set.hpp>            // basic_bit_bounded_set
#include <xstd/bits/bit_bounded_vector.hpp>         // basic_bit_bounded_vector
#include <xstd/bits/bit_key_mapping.hpp>            // bit_key_mapping
#include <xstd/bits/bit_fixed_set.hpp>              // basic_bit_fixed_set
#include <xstd/bits/bit_set.hpp>                    // basic_bit_set, bit_set
#include <xstd/bits/bit_set_view.hpp>               // bit_set_view
#include <xstd/bits/bit_type_traits/bit_rebind.hpp> // bit_rebind
#include <xstd/bits/bit_vector.hpp>                 // basic_bit_vector, bit_vector
#include <xstd/bits/ext/boost/bit_small_set.hpp>    // basic_bit_small_set
#include <xstd/bits/ext/boost/bit_small_vector.hpp> // basic_bit_small_vector, bit_small_vector
#include <boost/container/new_allocator.hpp>        // new_allocator
#include <boost/test/unit_test.hpp>                 // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                                // equal
#include <array>                                    // array
#include <bitset>                                   // bitset
#include <cstddef>                                  // size_t
#include <cstdint>                                  // uint16_t, uint32_t, uint64_t, uint8_t
#include <functional>                               // greater, less
#include <memory>                                   // allocator
#include <span>                                     // span
#include <type_traits>                              // is_same_v
#include <vector>                                   // vector

BOOST_AUTO_TEST_SUITE(BitTypeTraits)
BOOST_AUTO_TEST_SUITE(BitRebind)

namespace {

template<class Block, class W>
concept rebinds = requires { typename xstd::bit_rebind<Block, W>; };

} // namespace

// A width in the type stays as it is, and every other argument with it.
BOOST_AUTO_TEST_CASE(AFixedWidthOwnerKeepsItsWidth)
{
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint8_t, xstd::basic_bit_array<std::uint64_t, 9>>, xstd::basic_bit_array<std::uint8_t, 9>>);
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint16_t, xstd::basic_bit_bounded_vector<std::uint64_t, 9>>, xstd::basic_bit_bounded_vector<std::uint16_t, 9>>);
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint32_t, xstd::basic_bit_fixed_set<std::size_t, std::uint8_t, 9, xstd::bit_key_mapping<std::size_t>, std::greater<std::size_t>>>, xstd::basic_bit_fixed_set<std::size_t, std::uint32_t, 9, xstd::bit_key_mapping<std::size_t>, std::greater<std::size_t>>>); // NOLINT(modernize-use-transparent-functors): the comparator the type names
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint8_t, xstd::basic_bit_bounded_set<std::size_t, std::uint64_t, 9>>, xstd::basic_bit_bounded_set<std::size_t, std::uint8_t, 9>>);
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint64_t, xstd::basic_bit_array<std::uint64_t, 9>>, xstd::basic_bit_array<std::uint64_t, 9>>);
        BOOST_CHECK(true);
}

// An allocator over the old block is rebound to the new one, as std::allocator_traits::rebind_alloc does it.
BOOST_AUTO_TEST_CASE(AnAllocatorIsReboundWithTheBlock)
{
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint8_t, xstd::bit_vector>, xstd::basic_bit_vector<std::uint8_t, std::allocator<std::uint8_t>>>);
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint8_t, xstd::bit_set>, xstd::basic_bit_set<std::size_t, std::uint8_t, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, std::allocator<std::uint8_t>>>); // NOLINT(modernize-use-transparent-functors): the comparator the type names
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint16_t, xstd::bit_small_vector<9>>, xstd::basic_bit_small_vector<std::uint16_t, 9, boost::container::new_allocator<std::uint16_t>>>);
        static_assert(std::is_same_v<xstd::bit_rebind<std::uint32_t, xstd::basic_bit_small_set<std::size_t, std::uint8_t, 9>>, xstd::basic_bit_small_set<std::size_t, std::uint32_t, 9, xstd::bit_key_mapping<std::size_t>, std::less<std::size_t>, boost::container::new_allocator<std::uint32_t>>>); // NOLINT(modernize-use-transparent-functors): the comparator the type names
        BOOST_CHECK(true);
}

// The rebound owner holds the same bits, in blocks of the type it names.
BOOST_AUTO_TEST_CASE(TheReboundOwnerHoldsTheSameBits)
{
        using narrow_vector = xstd::bit_rebind<std::uint8_t, xstd::bit_vector>;
        static_assert(std::is_same_v<narrow_vector::block_container_type, std::vector<std::uint8_t>>);
        auto const wide   = xstd::bit_vector{true, false, true, true, false, false, false, false, true};
        auto const narrow = narrow_vector(wide.begin(), wide.end());
        BOOST_CHECK_EQUAL(narrow.size(), wide.size());
        BOOST_CHECK(std::ranges::equal(narrow, wide));
}

// A view, a standard bit container, a bare block and a range of blocks own no type to rebind.
BOOST_AUTO_TEST_CASE(OnlyAnOwnerIsRebound)
{
        static_assert(rebinds<std::uint8_t, xstd::bit_vector> and rebinds<std::uint8_t, xstd::bit_set>);
        static_assert(not rebinds<std::uint8_t, xstd::bit_set_view<std::span<std::uint32_t>>>);
        static_assert(not rebinds<std::uint8_t, std::bitset<9>> and not rebinds<std::uint8_t, std::uint64_t> and not rebinds<std::uint8_t, std::array<std::uint8_t, 2>>);
        static_assert(not rebinds<bool, xstd::bit_vector> and not rebinds<int, xstd::bit_vector>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
