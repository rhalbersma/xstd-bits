//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                               // check_equal, make, model, model_of, patterns, reference_array, reference_of, writes
#include <xstd/bits/bit_valarray/bit_bounded_valarray.hpp> // bit_bounded_valarray
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_mask_array.hpp>       // bit_mask_array
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // bit_valarray
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                                        // same_as
#include <cstddef>                                         // size_t
#include <ranges>                                          // iota
#include <type_traits>                                     // is_copy_constructible_v, is_default_constructible_v
#include <utility>                                         // declval
#include <valarray>                                        // valarray

BOOST_AUTO_TEST_SUITE(BitMaskArray)

using P = xstd::bit_mask_array<std::size_t>;

// Only a subscript makes one; a copy refers to the same elements, and a write through a const one writes them.
BOOST_AUTO_TEST_CASE(OnlyASubscriptMakesOneAndACopyRefersToTheSameElements)
{
        static_assert(not std::is_default_constructible_v<P>);
        static_assert(std::is_copy_constructible_v<P>);
        static_assert(std::same_as<P::value_type, bool>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::declval<xstd::bit_valarray const&>()]), P>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = std::declval<P const&>()), P const&>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = true), void>);
        static_assert(std::same_as<decltype(std::declval<P const&>() ^= std::declval<xstd::bit_valarray const&>()), void>);

        auto const a                                                              = test::valarray::patterns(70UZ)[4];
        auto v                                                                    = test::valarray::make<xstd::bit_valarray>(a);
        auto expected                                                             = test::valarray::reference_of(a);
        auto const mask                                                           = test::valarray::make<xstd::bit_valarray>(test::valarray::patterns(70UZ)[2]);
        auto const p                                                              = v[mask];
        expected[test::valarray::reference_of(test::valarray::patterns(70UZ)[2])] = true;
        auto const copy                                                           = p;
        copy                                                                      = true;
        test::valarray::check_equal(v, test::valarray::model_of(expected));
}

// A proxy refers to its array, as std::valarray's do, and writes through to it for as long as the array lives.
BOOST_AUTO_TEST_CASE(AProxyWritesThroughToItsArrayWhileTheArrayLives)
{
        auto const a         = test::valarray::patterns(70UZ)[5];
        auto v               = test::valarray::make<xstd::bit_valarray>(a);
        auto expected        = test::valarray::reference_of(a);
        auto const m         = test::valarray::patterns(70UZ)[3];
        auto const mask      = test::valarray::make<xstd::bit_valarray>(m);
        auto const p         = v[mask];
        auto const reference = test::valarray::reference_of(m);
        auto const r         = expected[reference];
        auto const n         = 35UZ;
        for (auto const w : test::valarray::writes) {
                test::valarray::apply(w, p, true);
                test::valarray::apply(w, r, test::valarray::reference_array(true, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
                test::valarray::apply(w, p, false);
                test::valarray::apply(w, r, test::valarray::reference_array(false, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
        }
}

// [mask.array.assign]: from another mask of the same array, every element is read before any is written.
BOOST_AUTO_TEST_CASE(AssigningAProxyAssignsItsElementsGatheredBeforeAnyIsWritten)
{
        auto const a  = test::valarray::patterns(70UZ)[4];
        auto v        = test::valarray::make<xstd::bit_valarray>(a);
        auto const lo = test::valarray::make<xstd::bit_valarray>(test::valarray::patterns(70UZ)[2]);
        auto const hi = test::valarray::make<xstd::bit_valarray>(test::valarray::patterns(70UZ)[3]);
        v[hi]         = v[lo];
        auto expected = a;
        for (auto const j : std::views::iota(0UZ, 35UZ)) {
                expected[(2UZ * j) + 1UZ] = a[2UZ * j];
        }
        test::valarray::check_equal(v, expected);

        // A mask made in the same full expression lives until the write is done.
        auto u        = test::valarray::make<xstd::bit_valarray>(a);
        u[u == false] = true;
        test::valarray::check_equal(u, test::valarray::model(70UZ, true));
}

// Every owner takes a proxy of any owner over the same blocks, at the fixed size only one of the type's own size.
BOOST_AUTO_TEST_CASE(EveryOwnerOverTheSameBlocksReadsAnotherOwnersProxy)
{
        auto const a = test::valarray::patterns(130UZ)[4];
        auto x       = test::valarray::make<xstd::bit_valarray>(a);
        auto const r = test::valarray::reference_of(a);
        auto m       = test::valarray::model(130UZ);
        for (auto const i : {1UZ, 9UZ, 17UZ, 64UZ, 65UZ, 100UZ, 128UZ, 129UZ}) {
                m[i] = true;
        }
        auto const mask = xstd::bit_bounded_valarray<130>(test::valarray::make<xstd::bit_valarray>(m)[std::slice(0UZ, 130UZ, 1UZ)]);
        test::valarray::check_equal(xstd::bit_fixed_valarray<8>(x[mask]), test::valarray::model_of(r[test::valarray::reference_of(m)]));
        test::valarray::check_equal(xstd::bit_bounded_valarray<64>(x[mask]), test::valarray::model_of(r[test::valarray::reference_of(m)]));
}

BOOST_AUTO_TEST_SUITE_END()
