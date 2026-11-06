//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                               // check_equal, make, model, model_of, patterns, reference_array, reference_of, writes
#include <xstd/bits/bit/bit_convert.hpp>                   // bit_convert
#include <xstd/bits/bit_span/bit_span.hpp>                 // bit_span
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp>   // bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_inplace_valarray.hpp> // bit_inplace_valarray
#include <xstd/bits/bit_valarray/bit_mask_array.hpp>       // bit_mask_array
#include <xstd/bits/bit_valarray/bit_valarray.hpp>         // bit_valarray
#include <xstd/bits/bit_vector.hpp>                        // bit_vector
#include <boost/test/unit_test.hpp>                        // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <concepts>                                        // same_as
#include <cstddef>                                         // size_t
#include <ranges>                                          // iota
#include <stdexcept>                                       // overflow_error
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
        auto const mask = xstd::bit_inplace_valarray<130>(test::valarray::make<xstd::bit_valarray>(m)[std::slice(0UZ, 130UZ, 1UZ)]);
        test::valarray::check_equal(xstd::bit_fixed_valarray<8>(x[mask]), test::valarray::model_of(r[test::valarray::reference_of(m)]));
        test::valarray::check_equal(xstd::bit_inplace_valarray<64>(x[mask]), test::valarray::model_of(r[test::valarray::reference_of(m)]));
}

// [valarray.sub] subscripts a std::valarray by a std::valarray<bool>, which bit_convert unpacks a mask into and packs back.
BOOST_AUTO_TEST_CASE(AMaskCrossesToAStdValarrayOfBoolAndSelectsTheSameElements)
{
        auto const a     = test::valarray::patterns(130UZ)[4];
        auto const m     = test::valarray::patterns(130UZ)[5];
        auto const check = [&]<class V>() -> void {
                auto const mask     = xstd::bit_convert<V>(test::valarray::reference_of(m));
                auto const unpacked = xstd::bit_convert<test::valarray::reference_array>(mask);
                test::valarray::check_equal(mask, m);
                BOOST_CHECK(test::valarray::model_of(unpacked) == m);

                auto const x = test::valarray::make<xstd::bit_valarray>(a);
                auto const r = test::valarray::reference_of(a);
                BOOST_CHECK(test::valarray::model_of(xstd::bit_valarray(x[mask])) == test::valarray::model_of(test::valarray::reference_array(r[unpacked])));
        };
        check.template operator()<xstd::bit_valarray>();
        check.template operator()<xstd::bit_fixed_valarray<130>>();
        check.template operator()<xstd::bit_inplace_valarray<130>>();
}

// Any source unpacks at its own width, a sequence and a window included; an empty one is an empty valarray.
BOOST_AUTO_TEST_CASE(EverySourceUnpacksAtItsWidth)
{
        auto const m = test::valarray::patterns(70UZ)[3];
        auto v       = xstd::bit_vector(m.size());
        for (auto const i : std::views::iota(0UZ, m.size())) {
                v[i] = m[i];
        }
        BOOST_CHECK(test::valarray::model_of(xstd::bit_convert<test::valarray::reference_array>(v)) == m);

        auto const window = xstd::bit_span(v).subspan(3UZ, 60UZ);
        auto const tail   = test::valarray::model(m.begin() + 3, m.begin() + 63);
        BOOST_CHECK(test::valarray::model_of(xstd::bit_convert<test::valarray::reference_array>(window)) == tail);

        BOOST_CHECK_EQUAL(xstd::bit_convert<test::valarray::reference_array>(xstd::bit_valarray()).size(), 0UZ);
        BOOST_CHECK(xstd::bit_convert<xstd::bit_valarray>(test::valarray::reference_array()).size() == 0UZ);
}

// Into a fixed width, as from any run-time source: a set position past it throws rather than drops.
BOOST_AUTO_TEST_CASE(AFixedMaskRefusesAPositionPastItsWidth)
{
        auto wide = test::valarray::reference_array(false, 70UZ);
        wide[69]  = true;
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_convert<xstd::bit_fixed_valarray<64>>(wide)), std::overflow_error);
        wide[69] = false;
        wide[63] = true;
        BOOST_CHECK(xstd::bit_convert<xstd::bit_fixed_valarray<64>>(wide)[63]);
}

BOOST_AUTO_TEST_SUITE_END()
