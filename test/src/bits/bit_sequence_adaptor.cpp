//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>            // basic_bit_array
#include <xstd/bits/bit_inplace_vector.hpp>   // IWYU pragma: keep; basic_bit_inplace_vector, named only under __cpp_lib_inplace_vector
#include <xstd/bits/bit_sequence_adaptor.hpp> // bit_sequence_adaptor
#include <xstd/bits/bit_vector.hpp>           // basic_bit_vector
#include <xstd/bits/from_bit_storage.hpp>     // from_bit_storage
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <array>                              // array
#include <concepts>                           // same_as
#include <cstdint>                            // uint8_t, uint16_t, uint32_t, uint64_t
#include <vector>                             // vector
#include <version>                            // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitSequenceAdaptor)

// Each owner is the adaptor over its storage, so the two spellings name one type.
BOOST_AUTO_TEST_CASE(OwnersAreTheAdaptorOverTheirStorage)
{
        static_assert(std::same_as<xstd::basic_bit_array<std::uint64_t, 64>, xstd::bit_sequence_adaptor<std::array<std::uint64_t, 1>>>);
        static_assert(std::same_as<xstd::basic_bit_array<std::uint8_t, 20>, xstd::bit_sequence_adaptor<std::array<std::uint8_t, 3>, 20>>);
        static_assert(std::same_as<xstd::basic_bit_vector<std::uint32_t>, xstd::bit_sequence_adaptor<std::vector<std::uint32_t>>>);
#ifdef __cpp_lib_inplace_vector
        static_assert(std::same_as<xstd::basic_bit_inplace_vector<std::uint16_t, 48>, xstd::bit_sequence_adaptor<std::inplace_vector<std::uint16_t, 3>>>);
#endif
        BOOST_CHECK(true);
}

// The adaptor's own name deduces the width from bits: an integer's digits, or an array's blocks of them.
BOOST_AUTO_TEST_CASE(TheAdaptorDeducesFromBits)
{
        constexpr auto word = xstd::bit_sequence_adaptor(xstd::from_bit_storage, std::uint16_t{0b1000'0000'0000'0101});
        static_assert(std::same_as<decltype(word), xstd::bit_sequence_adaptor<std::array<std::uint16_t, 1>, 16> const>);
        static_assert(word[0] and not word[1] and word[2] and word[15]);

        constexpr auto words = xstd::bit_sequence_adaptor(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x80});
        static_assert(std::same_as<decltype(words), xstd::bit_sequence_adaptor<std::array<std::uint8_t, 3>, 24> const>);
        static_assert(words[0] and words[23] and words.count() == 2UZ);
        BOOST_CHECK((words == xstd::basic_bit_array<std::uint8_t, 24>(xstd::from_bit_storage, std::array<std::uint8_t, 3>{0x01, 0x00, 0x80})));
}

// A word names its own storage: the layout of an array of one, at its digits or a narrower width.
BOOST_AUTO_TEST_CASE(AWordIsItsOwnStorage)
{
        auto word = xstd::bit_sequence_adaptor<std::uint16_t>();
        static_assert(sizeof(word) == sizeof(std::uint16_t));
        word[15] = true;
        BOOST_CHECK_EQUAL(word.to_bits<std::uint16_t>(), 0x8000U);
        BOOST_CHECK_EQUAL((xstd::bit_sequence_adaptor<std::uint8_t, 5>().size()), 5UZ);
}

BOOST_AUTO_TEST_SUITE_END()
