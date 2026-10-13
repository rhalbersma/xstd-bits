//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                // check_equal, make, model_of, patterns, reference_of
#include <xstd/bits/bit_fixed_valarray.hpp> // bit_fixed_valarray
#include <xstd/bits/bit_slice.hpp>          // bit_slice
#include <xstd/bits/bit_valarray.hpp>       // bit_valarray
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                         // regular
#include <type_traits>                      // is_trivially_copyable_v
#include <valarray>                         // slice

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
