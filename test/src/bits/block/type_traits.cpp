//          Copyright Rein Halbersma 2014-2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/uint128.hpp>         // TEST_HAS_UINT128, uint128
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_AUTO_TEST_CASE_TEMPLATE
#include <bit>                      // popcount
#include <cstdint>                  // uint8_t, uint16_t, uint32_t, uint64_t
#include <limits>                   // digits
#include <tuple>                    // tuple
#include <type_traits>              // is_integral_v, is_unsigned_v

BOOST_AUTO_TEST_SUITE(Block)
BOOST_AUTO_TEST_SUITE(TypeTraits)

using Types = std::tuple
<       uint8_t
,       uint16_t
,       uint32_t
,       uint64_t
#ifdef TEST_HAS_UINT128
,       xstd::uint128
#endif
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(IsUnsigned, T, Types)
{
        static_assert(std::is_unsigned_v<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(IsIntegral, T, Types)
{
        static_assert(std::is_integral_v<T>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DigitsEqualsSizeOf, T, Types)
{
        static_assert(std::numeric_limits<T>::digits == std::numeric_limits<unsigned char>::digits * sizeof(T));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DigitsEqualsCountLZero, T, Types)
{
        static_assert(std::numeric_limits<T>::digits == std::countl_zero(static_cast<T>(0)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DigitsEqualsCountRZero, T, Types)
{
        static_assert(std::numeric_limits<T>::digits == std::countr_zero(static_cast<T>(0)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DigitsEqualsPopCount, T, Types)
{
        static_assert(std::numeric_limits<T>::digits == std::popcount(static_cast<T>(-1)));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
