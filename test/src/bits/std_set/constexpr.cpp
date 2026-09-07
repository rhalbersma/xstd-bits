//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <compare>                      // strong_ordering
#include <cstdint>                      // uint8_t, uint16_t, uint32_t, uint64_t
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(Constexpr)


using Types = std::tuple
<       xstd::basic_bit_static_set<  0, uint8_t>
,       xstd::basic_bit_static_set<  1, uint8_t>
,       xstd::basic_bit_static_set<  7, uint8_t>
,       xstd::basic_bit_static_set<  8, uint8_t>
,       xstd::basic_bit_static_set<  9, uint8_t>
,       xstd::basic_bit_static_set< 15, uint8_t>
,       xstd::basic_bit_static_set< 16, uint8_t>
,       xstd::basic_bit_static_set< 17, uint8_t>
,       xstd::basic_bit_static_set< 24, uint8_t>
,       xstd::basic_bit_static_set<  0, uint16_t>
,       xstd::basic_bit_static_set<  1, uint16_t>
,       xstd::basic_bit_static_set< 15, uint16_t>
,       xstd::basic_bit_static_set< 16, uint16_t>
,       xstd::basic_bit_static_set< 17, uint16_t>
,       xstd::basic_bit_static_set< 31, uint16_t>
,       xstd::basic_bit_static_set< 32, uint16_t>
,       xstd::basic_bit_static_set< 33, uint16_t>
,       xstd::basic_bit_static_set< 48, uint16_t>
,       xstd::basic_bit_static_set<  0, uint32_t>
,       xstd::basic_bit_static_set<  1, uint32_t>
,       xstd::basic_bit_static_set< 31, uint32_t>
,       xstd::basic_bit_static_set< 32, uint32_t>
,       xstd::basic_bit_static_set< 33, uint32_t>
,       xstd::basic_bit_static_set< 63, uint32_t>
,       xstd::basic_bit_static_set< 64, uint32_t>
,       xstd::basic_bit_static_set< 65, uint32_t>
,       xstd::basic_bit_static_set<  0, uint64_t>
,       xstd::basic_bit_static_set<  1, uint64_t>
,       xstd::basic_bit_static_set< 63, uint64_t>
,       xstd::basic_bit_static_set< 64, uint64_t>
,       xstd::basic_bit_static_set< 65, uint64_t>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<  0, xstd::uint128>
,       xstd::basic_bit_static_set<  1, xstd::uint128>
,       xstd::basic_bit_static_set<127, xstd::uint128>
,       xstd::basic_bit_static_set<128, xstd::uint128>
,       xstd::basic_bit_static_set<129, xstd::uint128>
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
