//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                  // check_equal, make, model, model_of, patterns, reference_array, reference_of, write, writes
#include <xstd/bits/bit_bounded_valarray.hpp> // bit_bounded_valarray
#include <xstd/bits/bit_fixed_valarray.hpp>   // bit_fixed_valarray
#include <xstd/bits/bit_gslice_array.hpp>     // bit_gslice_array
#include <xstd/bits/bit_valarray.hpp>         // bit_valarray
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL, BOOST_TEST_CONTEXT
#include <concepts>                           // same_as
#include <cstddef>                            // size_t
#include <ranges>                             // iota
#include <type_traits>                        // is_copy_constructible_v, is_default_constructible_v
#include <utility>                            // declval
#include <valarray>                           // gslice, valarray

BOOST_AUTO_TEST_SUITE(BitGsliceArray)

using P = xstd::bit_gslice_array<std::size_t>;

// Only a subscript makes one; a copy refers to the same elements, and a write through a const one writes them.
BOOST_AUTO_TEST_CASE(OnlyASubscriptMakesOneAndACopyRefersToTheSameElements)
{
        static_assert(not std::is_default_constructible_v<P>);
        static_assert(std::is_copy_constructible_v<P>);
        static_assert(std::same_as<P::value_type, bool>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::gslice()]), P>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = std::declval<P const&>()), P const&>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = true), void>);
        static_assert(std::same_as<decltype(std::declval<P const&>() ^= std::declval<xstd::bit_valarray const&>()), void>);

        auto const a    = test::valarray::patterns(70UZ)[4];
        auto v          = test::valarray::make<xstd::bit_valarray>(a);
        auto expected   = test::valarray::reference_of(a);
        auto const g    = std::gslice(1UZ, std::valarray<std::size_t>{3UZ, 3UZ}, std::valarray<std::size_t>{20UZ, 2UZ});
        auto const p    = v[g];
        expected[g]     = true;
        auto const copy = p; // NOLINT(performance-unnecessary-copy-initialization): the copy is what is tested.
        copy            = true;
        test::valarray::check_equal(v, test::valarray::model_of(expected));
}

// A proxy refers to its array, as std::valarray's do, and writes through to it for as long as the array lives.
BOOST_AUTO_TEST_CASE(AProxyWritesThroughToItsArrayWhileTheArrayLives)
{
        auto const a  = test::valarray::patterns(70UZ)[5];
        auto v        = test::valarray::make<xstd::bit_valarray>(a);
        auto expected = test::valarray::reference_of(a);
        auto const g  = std::gslice(0UZ, std::valarray<std::size_t>{2UZ, 3UZ}, std::valarray<std::size_t>{1UZ, 9UZ});
        auto const p  = v[g];
        auto r        = expected[g];
        auto const n  = 6UZ;
        for (auto const w : test::valarray::writes) {
                test::valarray::apply(w, p, true);
                test::valarray::apply(w, r, test::valarray::reference_array(true, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
                test::valarray::apply(w, p, false);
                test::valarray::apply(w, r, test::valarray::reference_array(false, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
        }
}

// [gslice.array.assign]: from an overlapping gslice of the same array, every element is read before any is written.
BOOST_AUTO_TEST_CASE(AssigningAProxyAssignsItsElementsGatheredBeforeAnyIsWritten)
{
        auto const a  = test::valarray::patterns(70UZ)[4];
        auto v        = test::valarray::make<xstd::bit_valarray>(a);
        auto const g  = std::gslice(0UZ, std::valarray<std::size_t>{4UZ, 8UZ}, std::valarray<std::size_t>{8UZ, 1UZ});
        auto const h  = std::gslice(3UZ, std::valarray<std::size_t>{4UZ, 8UZ}, std::valarray<std::size_t>{8UZ, 1UZ});
        v[h]          = v[g];
        auto expected = a;
        for (auto const j : std::views::iota(0UZ, 32UZ)) {
                expected[3UZ + j] = a[j];
        }
        test::valarray::check_equal(v, expected);
}

// Every owner takes a proxy of any owner over the same blocks, at the fixed size only one of the type's own size.
BOOST_AUTO_TEST_CASE(EveryOwnerOverTheSameBlocksReadsAnotherOwnersProxy)
{
        auto const a = test::valarray::patterns(130UZ)[4];
        auto x       = test::valarray::make<xstd::bit_valarray>(a);
        auto const r = test::valarray::reference_of(a);
        auto const g = std::gslice(3UZ, std::valarray<std::size_t>{2UZ, 4UZ}, std::valarray<std::size_t>{65UZ, 2UZ});
        test::valarray::check_equal(xstd::bit_fixed_valarray<8>(x[g]), test::valarray::model_of(r[g]));
        test::valarray::check_equal(xstd::bit_bounded_valarray<64>(x[g]), test::valarray::model_of(r[g]));
}

// The paths agree: an ascending gslice takes the mask path, an index array of its positions the index path.
BOOST_AUTO_TEST_CASE(TheMaskPathAgreesWithTheIndexPath)
{
        auto const a       = test::valarray::patterns(130UZ)[4];
        auto const g       = std::gslice(5UZ, std::valarray<std::size_t>{2UZ, 3UZ, 4UZ}, std::valarray<std::size_t>{64UZ, 17UZ, 3UZ});
        auto const v       = test::valarray::make<xstd::bit_valarray>(a);
        auto const s       = test::valarray::gslice_indices(g);
        auto const indices = std::valarray<std::size_t>(s.data(), s.size());
        test::valarray::check_equal(v[g], test::valarray::model_of(v[indices]));
        auto x     = v;
        auto y     = v;
        x[g]       = ~v[g];
        y[indices] = ~v[indices];
        test::valarray::check_equal(x, test::valarray::model_of(y));
}

BOOST_AUTO_TEST_SUITE_END()
