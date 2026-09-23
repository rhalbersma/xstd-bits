//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>           // IWYU pragma: keep; TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_array.hpp>    // basic_bit_array
#include <xstd/bits/bit_sequence.hpp> // bit_sequence
#include <xstd/bits/bit_vector.hpp>   // basic_bit_vector
#include <xstd/bits/from_bits.hpp>    // from_bits
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                      // array
#include <concepts>                   // same_as
#include <cstdint>                    // int8_t, uint8_t, uint32_t, uint64_t
#include <deque>                      // deque
#include <memory_resource>            // polymorphic_allocator
#include <vector>                     // vector
#include <version>                    // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <xstd/bits/bit_inplace_vector.hpp> // basic_bit_inplace_vector
#include <inplace_vector>                   // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitSequence)

namespace {

template<class C>
concept names_a_sequence = requires { typename xstd::bit_sequence<C>; };

} // namespace

// Each storage of blocks names the owner that already holds such blocks: no type is new.
BOOST_AUTO_TEST_CASE(EachStorageNamesAnExistingOwner)
{
        static_assert(std::same_as<xstd::bit_sequence<std::uint64_t>, xstd::basic_bit_array<std::uint64_t, 64>>);
        static_assert(std::same_as<xstd::bit_sequence<std::uint8_t>, xstd::basic_bit_array<std::uint8_t, 8>>);
        static_assert(std::same_as<xstd::bit_sequence<std::array<std::uint32_t, 3>>, xstd::basic_bit_array<std::uint32_t, 96>>);
        static_assert(std::same_as<xstd::bit_sequence<std::vector<std::uint32_t>>, xstd::basic_bit_vector<std::uint32_t>>);
        static_assert(std::same_as<xstd::bit_sequence<std::pmr::vector<std::uint8_t>>, xstd::basic_bit_vector<std::uint8_t, std::pmr::polymorphic_allocator<std::uint8_t>>>);
#ifdef TEST_HAS_UINT128

        static_assert(std::same_as<xstd::bit_sequence<xstd::uint128>, xstd::basic_bit_array<xstd::uint128, 128>>);

#endif
#ifdef __cpp_lib_inplace_vector

        static_assert(std::same_as<xstd::bit_sequence<std::inplace_vector<std::uint8_t, 3>>, xstd::basic_bit_inplace_vector<std::uint8_t, 24>>);

#endif

        // The owner it names is the one the tagged guide deduces from the same bits.
        constexpr auto board = std::uint64_t{0x8000'0000'0000'0001};
        static_assert(std::same_as<xstd::bit_sequence<std::uint64_t>, decltype(xstd::basic_bit_array(xstd::from_bits, board))>);
        BOOST_CHECK(xstd::bit_sequence<std::uint64_t>::from_bits(board).count() == 2UZ);
}

// No sequence packs a signed word, a non-contiguous storage, or bool itself.
BOOST_AUTO_TEST_CASE(OnlyAStorageOfUnsignedBlocksNamesOne)
{
        static_assert(names_a_sequence<std::uint32_t>);
        static_assert(not names_a_sequence<int>);
        static_assert(not names_a_sequence<bool>);
        static_assert(not names_a_sequence<std::vector<int>>);
        static_assert(not names_a_sequence<std::deque<std::uint32_t>>);
        static_assert(not names_a_sequence<std::array<std::int8_t, 2>>);
        BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
