//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_CHECK_EQUAL_COLLECTIONS
#include <algorithm>                    // copy
#include <cstddef>                      // size_t
#include <cstdint>                      // uint16_t, uint32_t, uint64_t
#include <iterator>                     // inserter
#include <set>                          // set
#include <tuple>                        // tuple

BOOST_AUTO_TEST_SUITE(StdSet)
BOOST_AUTO_TEST_SUITE(Implicit)

using namespace test;

using Types = std::tuple
<       std::set<std::size_t>
#ifdef TEST_HAS_FLAT_SET
,       std::flat_set<std::size_t>
#endif
,       xstd::basic_bit_static_set<uint16_t, 32>
,       xstd::basic_bit_static_set<uint16_t, 33>
,       xstd::basic_bit_static_set<uint16_t, 48>
,       xstd::basic_bit_static_set<uint32_t, 32>
,       xstd::basic_bit_static_set<uint32_t, 33>
,       xstd::basic_bit_static_set<uint32_t, 64>
,       xstd::basic_bit_static_set<uint32_t, 65>
,       xstd::basic_bit_static_set<uint32_t, 96>
,       xstd::basic_bit_static_set<uint64_t, 64>
,       xstd::basic_bit_static_set<uint64_t, 65>
,       xstd::basic_bit_static_set<uint64_t, 128>
,       xstd::basic_bit_static_set<uint64_t, 129>
,       xstd::basic_bit_static_set<uint64_t, 192>
#ifdef TEST_HAS_UINT128
,       xstd::basic_bit_static_set<xstd::uint128, 128>
,       xstd::basic_bit_static_set<xstd::uint128, 129>
,       xstd::basic_bit_static_set<xstd::uint128, 256>
,       xstd::basic_bit_static_set<xstd::uint128, 257>
,       xstd::basic_bit_static_set<xstd::uint128, 384>
#endif
>;

class Implicit
{
        std::size_t m_value;

public:
        [[nodiscard]] constexpr explicit(false) Implicit(std::size_t v) noexcept : m_value(v) {}
        // Implicit is the point: this class exists to convert both ways without a cast.
        [[nodiscard]] constexpr explicit(false) operator std::size_t() const noexcept { return m_value; }  // NOLINT(misc-explicit-constructor)
};

BOOST_AUTO_TEST_CASE_TEMPLATE(TheKeysCopyIntoASetOfAnImplicitlyConstructibleType, T, Types)
{
        auto const src = T({ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31 });
        std::set<Implicit> dst;
        std::ranges::copy(src, std::inserter(dst, dst.end()));
        BOOST_CHECK_EQUAL_COLLECTIONS(src.begin(), src.end(), dst.begin(), dst.end());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
