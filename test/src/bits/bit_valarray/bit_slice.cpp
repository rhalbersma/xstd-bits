//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                             // check_equal, make, model_of, patterns, reference_of
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_slice.hpp>          // bit_slice
#include <xstd/bits/bit_valarray/bit_valarray.hpp>       // bit_valarray
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <algorithm>                                     // ranges::equal
#include <concepts>                                      // regular
#include <type_traits>                                   // is_convertible_v, is_trivially_copyable_v
#include <valarray>                                      // slice, valarray

BOOST_AUTO_TEST_SUITE(BitSlice)

// [slice.cons], [slice.access] and [slice.ops], all three usable in a constant expression.
BOOST_AUTO_TEST_CASE(ItHoldsAStartASizeAndAStrideAsStdSliceDoes)
{
        static_assert(std::regular<xstd::bit_slice>);
        static_assert(std::is_trivially_copyable_v<xstd::bit_slice>);
        static_assert(xstd::bit_slice() == xstd::bit_slice(0UZ, 0UZ, 0UZ));
        static_assert(xstd::bit_slice(1UZ, 2UZ, 3UZ).start() == 1UZ);
        static_assert(xstd::bit_slice(1UZ, 2UZ, 3UZ).size() == 2UZ);
        static_assert(xstd::bit_slice(1UZ, 2UZ, 3UZ).stride() == 3UZ);
        static_assert(xstd::bit_slice(1UZ, 2UZ, 3UZ) != xstd::bit_slice(1UZ, 2UZ, 4UZ));
        BOOST_CHECK(xstd::bit_slice(std::slice(4UZ, 5UZ, 6UZ)) == xstd::bit_slice(4UZ, 5UZ, 6UZ));
}

// Two slices are equal only when all three numbers are, each told apart on its own.
BOOST_AUTO_TEST_CASE(EachNumberAloneMakesTwoSlicesUnequal)
{
        auto const s = xstd::bit_slice(std::slice(1UZ, 2UZ, 3UZ));
        BOOST_CHECK(s != xstd::bit_slice(9UZ, 2UZ, 3UZ));
        BOOST_CHECK(s != xstd::bit_slice(1UZ, 9UZ, 3UZ));
        BOOST_CHECK(s != xstd::bit_slice(1UZ, 2UZ, 9UZ));
}

// It selects what the std::slice of the same numbers selects.
BOOST_AUTO_TEST_CASE(ItSelectsWhatTheSameStdSliceSelects)
{
        auto const a = test::valarray::patterns(130UZ)[4];
        auto const v = test::valarray::make<xstd::bit_valarray>(a);
        auto const r = test::valarray::reference_of(a);
        for (auto const& s : {std::slice(0UZ, 130UZ, 1UZ), std::slice(3UZ, 40UZ, 3UZ), std::slice(64UZ, 3UZ, 0UZ), std::slice(1UZ, 2UZ, 100UZ)}) {
                test::valarray::check_equal(v[xstd::bit_slice(s)], test::valarray::model_of(r[s]));
        }
}

// [valarray.sub]: a std::valarray takes it as the std::slice it converts to, which converts back to it unchanged.
BOOST_AUTO_TEST_CASE(ItSubscriptsAStdValarrayAsTheSameStdSlice)
{
        static_assert(std::is_convertible_v<xstd::bit_slice, std::slice>);
        static_assert(not std::is_convertible_v<std::slice, xstd::bit_slice>);
        auto const ints  = std::valarray<int>{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7};
        auto const bools = std::valarray<bool>{true, false, false, true, true, false, true, true, false, false, true, false, true, true};
        for (auto const& s : {std::slice(0UZ, 14UZ, 1UZ), std::slice(1UZ, 4UZ, 3UZ), std::slice(5UZ, 3UZ, 0UZ), std::slice(13UZ, 1UZ, 9UZ)}) {
                auto const b = xstd::bit_slice(s);
                BOOST_CHECK(xstd::bit_slice(std::slice(b)) == b);
                BOOST_CHECK(std::ranges::equal(std::valarray<int>(ints[b]), std::valarray<int>(ints[s])));
                BOOST_CHECK(std::ranges::equal(std::valarray<bool>(bools[b]), std::valarray<bool>(bools[s])));
                auto by_bit_slice = ints;
                by_bit_slice[b]   = -1;
                auto by_slice     = ints;
                by_slice[s]       = -1;
                BOOST_CHECK(std::ranges::equal(by_bit_slice, by_slice));
        }
}

// A slice's gather and its proxy's writes in a constant expression, which a std::slice cannot reach.
BOOST_AUTO_TEST_CASE(ItSubscriptsInConstantExpressions)
{
        static_assert([] -> bool {
                using V                            = xstd::bit_fixed_valarray<70>;
                auto v                             = V{};
                v[xstd::bit_slice(1UZ, 23UZ, 3UZ)] = true;
                v[xstd::bit_slice(4UZ, 2UZ, 3UZ)] ^= true;
                v[xstd::bit_slice(66UZ, 4UZ, 0UZ)] |= xstd::bit_valarray{false, false, false, true};
                auto const w = V(v[xstd::bit_slice(0UZ, 70UZ, 1UZ)]);
                return w[1] and not w[4] and not w[7] and w[10] and w[66] and w[67] and not w[68] and (v == w).min();
        }());
}

BOOST_AUTO_TEST_SUITE_END()
