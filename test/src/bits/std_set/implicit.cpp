//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/unit_test.hpp>     // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_CHECK_EQUAL_COLLECTIONS
#include <test/flat_set.hpp>            // IWYU pragma: keep; TEST_HAS_FLAT_SET
#include <test/uint128.hpp>             // TEST_HAS_UINT128, uint128
#include <xstd/bits/bit_static_set.hpp> // bit_static_set
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
,       xstd::bit_static_set< 32, uint16_t>
,       xstd::bit_static_set< 33, uint16_t>
,       xstd::bit_static_set< 48, uint16_t>
,       xstd::bit_static_set< 32, uint32_t>
,       xstd::bit_static_set< 33, uint32_t>
,       xstd::bit_static_set< 64, uint32_t>
,       xstd::bit_static_set< 65, uint32_t>
,       xstd::bit_static_set< 96, uint32_t>
,       xstd::bit_static_set< 64, uint64_t>
,       xstd::bit_static_set< 65, uint64_t>
,       xstd::bit_static_set<128, uint64_t>
,       xstd::bit_static_set<129, uint64_t>
,       xstd::bit_static_set<192, uint64_t>
#ifdef TEST_HAS_UINT128
,       xstd::bit_static_set<128, xstd::uint128>
,       xstd::bit_static_set<129, xstd::uint128>
,       xstd::bit_static_set<256, xstd::uint128>
,       xstd::bit_static_set<257, xstd::uint128>
,       xstd::bit_static_set<384, xstd::uint128>
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
