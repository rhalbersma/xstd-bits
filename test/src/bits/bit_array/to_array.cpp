//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_array.hpp>  // bit_array, to_bit_array
#include <boost/test/unit_test.hpp> // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                 // constructible_from, convertible_to

BOOST_AUTO_TEST_SUITE(BitArrayToArray)

// A built-in array converts explicitly, position by position, and only one of the array's own width.
BOOST_AUTO_TEST_CASE(ABuiltInArrayOfItsWidthConverts)
{
        constexpr bool c[3] = {true, false, true}; // NOLINT(modernize-avoid-c-arrays): the built-in array is what is converted.
        bool const m[3]     = {false, true, true}; // NOLINT(modernize-avoid-c-arrays): the built-in array is what is converted.
        auto const a        = xstd::bit_array<3>(c);
        auto const b        = xstd::bit_array<3>(m);
        BOOST_CHECK(a == xstd::bit_array<3>({true, false, true}));
        BOOST_CHECK(b == xstd::bit_array<3>({false, true, true}));
        static_assert(xstd::bit_array<3>(c) == xstd::to_bit_array(c));
        static_assert(not std::constructible_from<xstd::bit_array<3>, bool const(&)[2]>); // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::convertible_to<bool const(&)[3], xstd::bit_array<3>>);     // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::constructible_from<xstd::bit_array<0>, bool const(&)[1]>); // NOLINT(modernize-avoid-c-arrays)
}

BOOST_AUTO_TEST_SUITE_END()
