//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_bounded_set.hpp>  // IWYU pragma: keep; basic_bit_bounded_set, named only under __cpp_lib_inplace_vector
#include <xstd/bits/bit_fixed_set.hpp>    // basic_bit_fixed_set
#include <xstd/bits/bit_set.hpp>          // basic_bit_set
#include <xstd/bits/bit_set_adaptor.hpp>  // bit_set_adaptor
#include <xstd/bits/detail/ownership.hpp> // owned_bits_t
#include <xstd/bits/from_bit_storage.hpp> // from_bit_storage
#include <boost/test/unit_test.hpp>       // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <algorithm>                      // equal
#include <array>                          // array
#include <concepts>                       // same_as
#include <cstdint>                        // uint8_t, uint16_t, uint32_t, uint64_t
#include <vector>                         // vector
#include <version>                        // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitSetAdaptor)

// Dependent, so an operator that is missing makes this false rather than ill-formed.
template<class X, class Y>
constexpr bool compares_with = requires (X const& x, Y const& y) { x == y; };

// Each owner wraps the storage the adaptor over it wraps, under a name of its own.
BOOST_AUTO_TEST_CASE(OwnersWrapTheStorageOfTheAdaptorOverIt)
{
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_fixed_set<std::uint64_t, 64>>, xstd::bits::detail::owned_bits_t<xstd::bit_set_adaptor<std::array<std::uint64_t, 1>>>>);
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_fixed_set<std::uint8_t, 20>>, xstd::bits::detail::owned_bits_t<xstd::bit_set_adaptor<std::array<std::uint8_t, 3>, 20>>>);
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_set<std::uint32_t>>, xstd::bits::detail::owned_bits_t<xstd::bit_set_adaptor<std::vector<std::uint32_t>>>>);
#ifdef __cpp_lib_inplace_vector
        static_assert(std::same_as<xstd::bits::detail::owned_bits_t<xstd::basic_bit_bounded_set<std::uint16_t, 48>>, xstd::bits::detail::owned_bits_t<xstd::bit_set_adaptor<std::inplace_vector<std::uint16_t, 3>>>>);
#endif

        // Two names over one storage are two types, and no comparison crosses between them.
        static_assert(not compares_with<xstd::basic_bit_fixed_set<std::uint8_t, 24>, xstd::bit_set_adaptor<std::array<std::uint8_t, 3>>>);
        static_assert(not compares_with<xstd::basic_bit_set<std::uint32_t>, xstd::bit_set_adaptor<std::vector<std::uint32_t>>>);
        BOOST_CHECK(true);
}

// The adaptor's own name deduces the width from bits: an integer's digits, or an array's blocks of them.
BOOST_AUTO_TEST_CASE(TheAdaptorDeducesFromBits)
{
        constexpr auto word = xstd::bit_set_adaptor(xstd::from_bit_storage, std::uint16_t{0b1000'0000'0000'0101});
        static_assert(std::same_as<decltype(word), xstd::bit_set_adaptor<std::array<std::uint16_t, 1>, 16> const>);
        static_assert(word.contains(0UZ) and not word.contains(1UZ) and word.contains(2UZ) and word.contains(15UZ));

        constexpr auto words = xstd::bit_set_adaptor(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x80});
        static_assert(std::same_as<decltype(words), xstd::bit_set_adaptor<std::array<std::uint8_t, 3>, 24> const>);
        static_assert(words.contains(0UZ) and words.contains(23UZ) and words.size() == 2UZ);
        BOOST_CHECK(std::ranges::equal(words, xstd::basic_bit_fixed_set<std::uint8_t, 24>(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x80})));
}

// A word names its own storage: the layout of an array of one, at its digits or a narrower width.
BOOST_AUTO_TEST_CASE(AWordIsItsOwnStorage)
{
        auto board = xstd::bit_set_adaptor<std::uint64_t>();
        static_assert(sizeof(board) == sizeof(std::uint64_t));
        BOOST_CHECK(board.insert(63UZ).second);
        BOOST_CHECK_EQUAL(board.to_bits<std::uint64_t>(), std::uint64_t{1} << 63U);
        BOOST_CHECK_EQUAL((xstd::bit_set_adaptor<std::uint8_t, 5>().max_size()), 5UZ);
}

BOOST_AUTO_TEST_SUITE_END()
