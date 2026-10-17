//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <xstd/bits/bit_valarray.hpp> // basic_bit_fixed_valarray, basic_bit_inplace_valarray, basic_bit_valarray, bit_fixed_valarray, bit_gslice, bit_gslice_array, bit_indirect_array, bit_inplace_valarray, bit_mask_array, bit_slice, bit_slice_array, bit_valarray
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL
#include <concepts>                   // same_as
#include <cstddef>                    // size_t
#include <utility>                    // declval
#include <valarray>                   // gslice, slice, valarray

BOOST_AUTO_TEST_SUITE(Valarray)

// The umbrella reaches every valarray, both selectors and the four proxies the subscripts return in one include.
BOOST_AUTO_TEST_CASE(TheUmbrellaReachesEveryValarrayAndProxy)
{
        static_assert(std::same_as<xstd::bit_fixed_valarray<8>, xstd::basic_bit_fixed_valarray<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_inplace_valarray<8>, xstd::basic_bit_inplace_valarray<std::size_t, 8>>);
        static_assert(std::same_as<xstd::bit_valarray, xstd::basic_bit_valarray<std::size_t>>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::slice()]), xstd::bit_slice_array<std::size_t>>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::gslice()]), xstd::bit_gslice_array<std::size_t>>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::declval<xstd::bit_valarray const&>()]), xstd::bit_mask_array<std::size_t>>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::declval<std::valarray<std::size_t> const&>()]), xstd::bit_indirect_array<std::size_t>>);
        static_assert(xstd::bit_slice(1UZ, 2UZ, 3UZ).size() == 2UZ);
        BOOST_CHECK_EQUAL(xstd::bit_gslice().start(), 0UZ);
}

BOOST_AUTO_TEST_SUITE_END()
