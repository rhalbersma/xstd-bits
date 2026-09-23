//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bitset.hpp>         // basic_bitset
#include <xstd/bits/bitset_adaptor.hpp> // bitset_adaptor
#include <xstd/bits/dynamic_bitset.hpp> // basic_dynamic_bitset
#include <xstd/bits/from_bits.hpp>      // from_bits
#include <xstd/bits/inplace_bitset.hpp> // IWYU pragma: keep; basic_inplace_bitset, named only under __cpp_lib_inplace_vector
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <array>                        // array
#include <concepts>                     // same_as
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <vector>                       // vector
#include <version>                      // IWYU pragma: keep; __cpp_lib_inplace_vector

#ifdef __cpp_lib_inplace_vector

#include <inplace_vector> // inplace_vector

#endif

BOOST_AUTO_TEST_SUITE(BitsetAdaptor)

// Each owner is the adaptor over its storage, so the two spellings name one type.
BOOST_AUTO_TEST_CASE(OwnersAreTheAdaptorOverTheirStorage)
{
        static_assert(std::same_as<xstd::basic_bitset<std::uint64_t, 64>, xstd::bitset_adaptor<std::array<std::uint64_t, 1>>>);
        static_assert(std::same_as<xstd::basic_bitset<std::uint8_t, 20>, xstd::bitset_adaptor<std::array<std::uint8_t, 3>, 20>>);
        static_assert(std::same_as<xstd::basic_dynamic_bitset<std::uint32_t>, xstd::bitset_adaptor<std::vector<std::uint32_t>>>);
#ifdef __cpp_lib_inplace_vector
        static_assert(std::same_as<xstd::basic_inplace_bitset<std::uint16_t, 48>, xstd::bitset_adaptor<std::inplace_vector<std::uint16_t, 3>>>);
#endif
        BOOST_CHECK(true);
}

// The adaptor's own name deduces the width from bits: an integer's digits, or an array's blocks of them.
BOOST_AUTO_TEST_CASE(TheAdaptorDeducesFromBits)
{
        constexpr auto word = xstd::bitset_adaptor(xstd::from_bits, std::uint16_t{0b1000'0000'0000'0101});
        static_assert(std::same_as<decltype(word), xstd::bitset_adaptor<std::array<std::uint16_t, 1>, 16> const>);
        static_assert(word.test(0UZ) and not word.test(1UZ) and word.test(2UZ) and word.test(15UZ));

        constexpr auto words = xstd::bitset_adaptor(xstd::from_bits, std::array<std::uint8_t, 3>{0x01, 0x00, 0x80});
        static_assert(std::same_as<decltype(words), xstd::bitset_adaptor<std::array<std::uint8_t, 3>, 24> const>);
        static_assert(words.test(0UZ) and words.test(23UZ) and words.count() == 2UZ);
        BOOST_CHECK((words == xstd::basic_bitset<std::uint8_t, 24>::from_bits(std::array<std::uint8_t, 3>{0x01, 0x00, 0x80})));
}

BOOST_AUTO_TEST_SUITE_END()
