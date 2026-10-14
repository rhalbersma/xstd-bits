//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                  // check_equal, make, model, model_of, patterns, reference_array, reference_of, write, writes
#include <xstd/bits/bit_bounded_valarray.hpp> // bit_bounded_valarray
#include <xstd/bits/bit_fixed_valarray.hpp>   // bit_fixed_valarray
#include <xstd/bits/bit_indirect_array.hpp>   // bit_indirect_array
#include <xstd/bits/bit_valarray.hpp>         // bit_valarray
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL, BOOST_TEST_CONTEXT
#include <concepts>                           // same_as
#include <cstddef>                            // size_t
#include <ranges>                             // iota
#include <type_traits>                        // is_copy_constructible_v, is_default_constructible_v
#include <utility>                            // declval
#include <valarray>                           // slice, valarray

BOOST_AUTO_TEST_SUITE(BitIndirectArray)

using P = xstd::bit_indirect_array<std::size_t>;

// Only a subscript makes one; a copy refers to the same elements, and a write through a const one writes them.
BOOST_AUTO_TEST_CASE(OnlyASubscriptMakesOneAndACopyRefersToTheSameElements)
{
        static_assert(not std::is_default_constructible_v<P>);
        static_assert(std::is_copy_constructible_v<P>);
        static_assert(std::same_as<P::value_type, bool>);
        static_assert(std::same_as<decltype(std::declval<xstd::bit_valarray&>()[std::declval<std::valarray<std::size_t> const&>()]), P>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = std::declval<P const&>()), P const&>);
        static_assert(std::same_as<decltype(std::declval<P const&>() = true), void>);
        static_assert(std::same_as<decltype(std::declval<P const&>() ^= std::declval<xstd::bit_valarray const&>()), void>);

        auto const a       = test::valarray::patterns(70UZ)[4];
        auto v             = test::valarray::make<xstd::bit_valarray>(a);
        auto expected      = test::valarray::reference_of(a);
        auto const indices = std::valarray<std::size_t>{69UZ, 0UZ, 33UZ, 64UZ};
        auto const p       = v[indices];
        expected[indices]  = true;
        auto const copy    = p;
        copy               = true;
        test::valarray::check_equal(v, test::valarray::model_of(expected));
}

// A proxy refers to its array, as std::valarray's do, and writes through to it for as long as the array lives.
BOOST_AUTO_TEST_CASE(AProxyWritesThroughToItsArrayWhileTheArrayLives)
{
        auto const a       = test::valarray::patterns(70UZ)[5];
        auto v             = test::valarray::make<xstd::bit_valarray>(a);
        auto expected      = test::valarray::reference_of(a);
        auto const indices = std::valarray<std::size_t>{5UZ, 1UZ, 66UZ, 2UZ, 64UZ, 63UZ};
        auto const p       = v[indices];
        auto const r       = expected[indices];
        auto const n       = 6UZ;
        for (auto const w : test::valarray::writes) {
                test::valarray::apply(w, p, true);
                test::valarray::apply(w, r, test::valarray::reference_array(true, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
                test::valarray::apply(w, p, false);
                test::valarray::apply(w, r, test::valarray::reference_array(false, n));
                test::valarray::check_equal(v, test::valarray::model_of(expected));
        }
}

// [indirect.array.assign]: from overlapping indices into the same array, every element is read before any is written.
BOOST_AUTO_TEST_CASE(AssigningAProxyAssignsItsElementsGatheredBeforeAnyIsWritten)
{
        auto const a    = test::valarray::patterns(70UZ)[4];
        auto v          = test::valarray::make<xstd::bit_valarray>(a);
        auto const to   = std::valarray<std::size_t>{1UZ, 2UZ, 3UZ, 4UZ};
        auto const from = std::valarray<std::size_t>{0UZ, 1UZ, 2UZ, 3UZ};
        v[to]           = v[from];
        auto expected   = a;
        for (auto const j : std::views::iota(0UZ, 4UZ)) {
                expected[j + 1UZ] = a[j];
        }
        test::valarray::check_equal(v, expected);
}

// Every owner takes a proxy of any owner over the same blocks, at the fixed size only one of the type's own size.
BOOST_AUTO_TEST_CASE(EveryOwnerOverTheSameBlocksReadsAnotherOwnersProxy)
{
        auto const a       = test::valarray::patterns(130UZ)[4];
        auto x             = test::valarray::make<xstd::bit_valarray>(a);
        auto const r       = test::valarray::reference_of(a);
        auto const indices = std::valarray<std::size_t>{129UZ, 0UZ, 64UZ, 63UZ, 7UZ, 7UZ, 128UZ, 1UZ};
        test::valarray::check_equal(xstd::bit_fixed_valarray<8>(x[indices]), test::valarray::model_of(r[indices]));
        test::valarray::check_equal(xstd::bit_bounded_valarray<64>(x[indices]), test::valarray::model_of(r[indices]));
}

BOOST_AUTO_TEST_SUITE_END()
