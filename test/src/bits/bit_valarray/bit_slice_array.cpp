//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                               // check_equal, make, model, model_of, patterns, reference_array, reference_of, write, writes
#include <xstd/bits/bit_valarray/bit_bounded_valarray.hpp> // bit_bounded_valarray
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_slice_array.hpp>      // bit_slice_array
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // bit_valarray
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL, BOOST_TEST_CONTEXT
#include <concepts>                                        // same_as
#include <cstddef>                                         // size_t
#include <ranges>                                          // iota
#include <type_traits>                                     // is_copy_constructible_v, is_default_constructible_v
#include <utility>                                         // declval, to_underlying
#include <valarray>                                        // slice

BOOST_AUTO_TEST_SUITE(BitSliceArray)

using P = xstd::bit_slice_array<std::size_t>;

// Only a subscript makes one; a copy refers to the same elements, and a write through a const one writes them.
BOOST_AUTO_TEST_CASE(OnlyASubscriptMakesOneAndACopyRefersToTheSameElements)
{
        static_assert(not std::is_default_constructible_v<P>);
        static_assert(std::is_copy_constructible_v<P>);
        static_assert(std::same_as<P::value_type, bool>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::slice()]), P>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = std::declval<P const&>()), P const&>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = true), void>);
        static_assert(std::same_as<decltype(std::declval<P const&>() ^= std::declval<xstd::bit_valarray const&>()), void>);

        auto const a                        = test::valarray::patterns(70UZ)[4];
        auto v                              = test::valarray::make<xstd::bit_valarray>(a);
        auto expected                       = test::valarray::reference_of(a);
        auto const p                        = v[std::slice(1UZ, 9UZ, 7UZ)];
        expected[std::slice(1UZ, 9UZ, 7UZ)] = true;
        auto const copy                     = p;
        copy                                = true;
        test::valarray::check_equal(v, test::valarray::model_of(expected));
}

// A proxy refers to its array, as std::valarray's do, and writes through to it for as long as the array lives.
BOOST_AUTO_TEST_CASE(AProxyWritesThroughToItsArrayWhileTheArrayLives)
{
        auto const a  = test::valarray::patterns(70UZ)[5];
        auto v        = test::valarray::make<xstd::bit_valarray>(a);
        auto expected = test::valarray::reference_of(a);
        auto const p  = v[std::slice(2UZ, 20UZ, 3UZ)];
        auto const r  = expected[std::slice(2UZ, 20UZ, 3UZ)];
        auto const n  = 20UZ;
        for (auto const w : test::valarray::writes) {
                test::valarray::apply(w, p, true);
                test::valarray::apply(w, r, test::valarray::reference_array(true, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
                test::valarray::apply(w, p, false);
                test::valarray::apply(w, r, test::valarray::reference_array(false, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
        }
}

// [slice.arr.assign]: from an overlapping slice of the same array, every element is read before any is written.
BOOST_AUTO_TEST_CASE(AssigningAProxyAssignsItsElementsGatheredBeforeAnyIsWritten)
{
        auto const a                  = test::valarray::patterns(70UZ)[4];
        auto v                        = test::valarray::make<xstd::bit_valarray>(a);
        v[std::slice(2UZ, 60UZ, 1UZ)] = v[std::slice(0UZ, 60UZ, 1UZ)];
        auto expected                 = a;
        for (auto const j : std::views::iota(0UZ, 60UZ)) {
                expected[2UZ + j] = a[j];
        }
        test::valarray::check_equal(v, expected);

        // The whole array onto itself, whose blocks the assignment would otherwise copy over themselves.
        auto u                        = test::valarray::make<xstd::bit_valarray>(a);
        u[std::slice(0UZ, 70UZ, 1UZ)] = u;
        test::valarray::check_equal(u, a);
}

// Every owner takes a proxy of any owner over the same blocks, at the fixed size only one of the type's own size.
BOOST_AUTO_TEST_CASE(EveryOwnerOverTheSameBlocksReadsAnotherOwnersProxy)
{
        auto const a = test::valarray::patterns(130UZ)[4];
        auto const v = test::valarray::make<xstd::bit_valarray>(a);
        auto x       = v;
        auto const r = test::valarray::reference_of(a);
        test::valarray::check_equal(xstd::bit_fixed_valarray<8>(x[std::slice(3UZ, 8UZ, 9UZ)]), test::valarray::model_of(r[std::slice(3UZ, 8UZ, 9UZ)]));
        test::valarray::check_equal(xstd::bit_bounded_valarray<64>(x[std::slice(5UZ, 40UZ, 3UZ)]), test::valarray::model_of(r[std::slice(5UZ, 40UZ, 3UZ)]));
}

// A slice of any stride class takes any owner over the same blocks as its array, the fixed one included.
BOOST_AUTO_TEST_CASE(ASliceTakesAnyOwnerOverTheSameBlocks)
{
        auto const a = test::valarray::patterns(460UZ)[4];
        auto const b = test::valarray::patterns(8UZ)[5];
        auto const f = test::valarray::make<xstd::bit_fixed_valarray<8>>(b);
        for (auto const& s : {std::slice(0UZ, 8UZ, 1UZ), std::slice(5UZ, 8UZ, 13UZ), std::slice(3UZ, 8UZ, 65UZ), std::slice(1UZ, 8UZ, 64UZ)}) {
                for (auto const w : test::valarray::writes) {
                        BOOST_TEST_CONTEXT("slice(" << s.start() << ", " << s.size() << ", " << s.stride() << "), write " << static_cast<int>(std::to_underlying(w)))
                        {
                                auto v        = test::valarray::make<xstd::bit_valarray>(a);
                                auto expected = test::valarray::reference_of(a);
                                test::valarray::apply(w, v[s], f);
                                test::valarray::apply(w, expected[s], test::valarray::reference_of(b));
                                test::valarray::check_equal(v, test::valarray::model_of(expected));
                        }
                }
        }
}

BOOST_AUTO_TEST_SUITE_END()
