//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <compare>                      // strong_ordering
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(Constexpr)


using Types = std::tuple
<       xstd::basic_bit_static_set<uint8_t, 0>
,       xstd::basic_bit_static_set<uint8_t, 1>
,       xstd::basic_bit_static_set<uint8_t, 7>
,       xstd::basic_bit_static_set<uint8_t, 8>
,       xstd::basic_bit_static_set<uint8_t, 9>
,       xstd::basic_bit_static_set<uint8_t, 15>
,       xstd::basic_bit_static_set<uint8_t, 16>
,       xstd::basic_bit_static_set<uint8_t, 17>
,       xstd::basic_bit_static_set<uint8_t, 24>
,       xstd::basic_bit_static_set<uint16_t, 0>
,       xstd::basic_bit_static_set<uint16_t, 1>
,       xstd::basic_bit_static_set<uint16_t, 15>
,       xstd::basic_bit_static_set<uint16_t, 16>
,       xstd::basic_bit_static_set<uint16_t, 17>
,       xstd::basic_bit_static_set<uint16_t, 31>
,       xstd::basic_bit_static_set<uint16_t, 32>
,       xstd::basic_bit_static_set<uint16_t, 33>
,       xstd::basic_bit_static_set<uint16_t, 48>
,       xstd::basic_bit_static_set<uint32_t, 0>
,       xstd::basic_bit_static_set<uint32_t, 1>
,       xstd::basic_bit_static_set<uint32_t, 31>
,       xstd::basic_bit_static_set<uint32_t, 32>
,       xstd::basic_bit_static_set<uint32_t, 33>
,       xstd::basic_bit_static_set<uint32_t, 63>
,       xstd::basic_bit_static_set<uint32_t, 64>
,       xstd::basic_bit_static_set<uint32_t, 65>
,       xstd::basic_bit_static_set<uint64_t, 0>
,       xstd::basic_bit_static_set<uint64_t, 1>
,       xstd::basic_bit_static_set<uint64_t, 63>
,       xstd::basic_bit_static_set<uint64_t, 64>
,       xstd::basic_bit_static_set<uint64_t, 65>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<xstd::uint128, 0>
,       xstd::basic_bit_static_set<xstd::uint128, 1>
,       xstd::basic_bit_static_set<xstd::uint128, 127>
,       xstd::basic_bit_static_set<xstd::uint128, 128>
,       xstd::basic_bit_static_set<xstd::uint128, 129>
#endif
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(AnEmptySetIsUsableInAConstantExpression, T, Types)
{
        constexpr auto b = T();
        static_assert(b.empty());
        static_assert(b.size() == 0);
        static_assert(b.begin() == b.end());
        // Reflexivity cannot be written without naming the object twice. [design.md#clang-tidy-false-positives]
        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AFullSetIsUsableInAConstantExpression, T, Types)
{
        constexpr auto b = ~T();
        static_assert(b.full());
        static_assert(b.size() == b.max_size());
        static_assert(b.empty() or b.front() == *b.cbegin());
        static_assert(b.empty() or b.back()  == *b.crbegin());
        // Reflexivity cannot be written without naming the object twice. [design.md#clang-tidy-false-positives]
        static_assert(b == b);                                   // NOLINT(misc-redundant-expression)
        static_assert((b <=> b) == std::strong_ordering::equal); // NOLINT(misc-redundant-expression)
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
