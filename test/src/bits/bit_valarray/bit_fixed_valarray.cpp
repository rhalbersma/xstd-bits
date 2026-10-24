//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>                          // graded_extents, wide_extents
#include <test/structural.hpp>                           // structural
#include <test/valarray.hpp>                             // check_*
#include <xstd/bits/bit_valarray/bit_fixed_valarray.hpp> // basic_bit_fixed_valarray, bit_fixed_valarray
#include <xstd/bits/bit_valarray/bit_valarray.hpp>       // bit_valarray
#include <xstd/bits/detail/inplace_blocks.hpp>           // IWYU pragma: keep; XSTD_BITS_HAS_CONSTEXPR_INPLACE
#include <boost/test/unit_test.hpp>                      // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK
#include <concepts>                                      // constructible_from, convertible_to, same_as
#include <cstddef>                                       // size_t
#include <cstdint>                                       // uint8_t
#include <tuple>                                         // tuple_cat
#include <utility>                                       // declval

BOOST_AUTO_TEST_SUITE(BitFixedValarray)

// Every Block model within one block, the narrow ones across boundaries, and the widest across one too.
using Types = decltype(std::tuple_cat(
        std::declval<test::graded_extents<xstd::basic_bit_fixed_valarray>>(),
        std::declval<test::wide_extents<xstd::basic_bit_fixed_valarray>>()
));

BOOST_AUTO_TEST_CASE_TEMPLATE(TheTypeIsShapedLikeAValarrayAndNotAContainer, V, Types)
{
        test::valarray::check_type<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ADefaultArrayIsEmptyOrItsSizeOfFalse, V, Types)
{
        test::valarray::check_default<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ACountConstructsThatManyOfTheValue, V, Types)
{
        test::valarray::check_count<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AListConstructsItsElementsInOrder, V, Types)
{
        test::valarray::check_list<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BlocksConstructTheirBitsAsElements, V, Types)
{
        test::valarray::check_from_blocks<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AssignmentTakesTheOtherSizeAndAScalarFills, V, Types)
{
        test::valarray::check_assign<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheSubscriptWritesAndReadsOneElement, V, Types)
{
        test::valarray::check_subscript<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SumAndMaxAreOrAndMinIsAnd, V, Types)
{
        test::valarray::check_reductions<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ShiftFillsWithFalseAndCshiftWrapsRound, V, Types)
{
        test::valarray::check_shift<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ApplyMapsEveryElement, V, Types)
{
        test::valarray::check_apply<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ResizeAssignsTheValueToEveryElement, V, Types)
{
        test::valarray::check_resize<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BothSwapsExchangeValuesAndSizes, V, Types)
{
        test::valarray::check_swap<V>();
}

BOOST_AUTO_TEST_CASE(TheDefaultBlockIsSizeT)
{
        static_assert(std::same_as<xstd::bit_fixed_valarray<3>, xstd::basic_bit_fixed_valarray<std::size_t, 3>>);
}

// Not std::valarray's default: every element false even when default-initialized, as writing one bool reads its block.
BOOST_AUTO_TEST_CASE(ADefaultInitializedArrayIsAllFalse)
{
        static_assert([] {
                xstd::bit_fixed_valarray<100> v;
                v[99] = true;
                return v.sum() and not v[0];
        }());
        xstd::bit_fixed_valarray<100> const v;
        BOOST_CHECK(not v.sum());
}

// Not in [valarray]: a built-in array of bools of the array's own size converts explicitly, as bit_array's does.
BOOST_AUTO_TEST_CASE(ABuiltInArrayOfItsSizeConverts)
{
        constexpr bool c[3] = {true, false, true}; // NOLINT(modernize-avoid-c-arrays): the built-in array is what is converted.
        static_assert(xstd::bit_fixed_valarray<3>(c)[2]);
        static_assert(not xstd::bit_fixed_valarray<3>(c)[1]);
        static_assert(not std::constructible_from<xstd::bit_fixed_valarray<3>, bool const(&)[2]>); // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::convertible_to<bool const(&)[3], xstd::bit_fixed_valarray<3>>);     // NOLINT(modernize-avoid-c-arrays)
        static_assert(not std::constructible_from<xstd::bit_fixed_valarray<3>, std::size_t>);
}

// Every member is constexpr, so an array is computed in a constant expression.
BOOST_AUTO_TEST_CASE(ItIsUsableInConstantExpressions)
{
        static_assert([] -> bool {
                auto v = xstd::bit_fixed_valarray<70>{true, false, true};
                v <<= 64UZ;
                auto const w = (v >> 64UZ) == xstd::bit_fixed_valarray<70>{true, false, true};
                return w.min() and v[66] and (v.shift(64) ^ ~v.cshift(-6)).sum();
        }());
}

// A mask's proxy writes, and an array is read from a pointer and from a proxy, in a constant expression.
BOOST_AUTO_TEST_CASE(TheMaskProxyIsUsableInConstantExpressions)
{
        static_assert([] -> bool {
                using V   = xstd::bit_fixed_valarray<70>;
                auto mask = V{true, false, true};
                mask[66]  = true;
                auto v    = V{};
                v[mask]   = xstd::bit_valarray{true, false, true};
                v[mask] ^= true;
                v[mask] |= xstd::bit_valarray{false, true, false};
                bool const bools[70] = {true, false, true}; // NOLINT(modernize-avoid-c-arrays): what the pointer reads.
                auto const all       = V(bools, 70UZ) | true;
                auto const u         = V(v[all]);
                return not u[0] and u[2] and not u[66] and (v == u).min() and V(bools, 70UZ)[2];
        }());
}

#ifdef XSTD_BITS_HAS_CONSTEXPR_INPLACE

// The const subscript gathers into the inplace array, which a constant expression holds where its blocks are inline.
BOOST_AUTO_TEST_CASE(TheMaskGatherIsUsableInConstantExpressions)
{
        static_assert([] -> bool {
                using V             = xstd::bit_fixed_valarray<70>;
                auto mask           = V{true, false, true};
                mask[66]            = true;
                auto const v        = V{true, true} | V{true}.shift(-66);
                auto const gathered = v[mask];
                return gathered.size() == 3UZ and gathered[0] and not gathered[1] and gathered[2];
        }());
}

#endif

// At a whole number of blocks every bit is a position, so the array is a structural type, as bit_array is.
BOOST_AUTO_TEST_CASE(AtAWholeNumberOfBlocksItIsStructural)
{
        static_assert(test::structural<xstd::bit_fixed_valarray<64>>);
        static_assert(test::structural<xstd::basic_bit_fixed_valarray<std::uint8_t, 16>>);
        static_assert(not test::structural<xstd::bit_fixed_valarray<3>>);
}

BOOST_AUTO_TEST_SUITE_END()
