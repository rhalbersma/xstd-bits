//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>             // graded_extents, wide_extents
#include <test/structural.hpp>              // structural
#include <test/valarray.hpp>                // check_*
#include <xstd/bits/bit_fixed_valarray.hpp> // basic_bit_fixed_valarray, bit_fixed_valarray
#include <boost/test/unit_test.hpp>         // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END
#include <concepts>                         // constructible_from, convertible_to, same_as
#include <cstddef>                          // size_t
#include <cstdint>                          // uint8_t
#include <tuple>                            // tuple_cat
#include <utility>                          // declval

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

BOOST_AUTO_TEST_CASE_TEMPLATE(NotAndComplementBothNegateEveryElement, V, Types)
{
        test::valarray::check_unary<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompoundAssignmentCombinesElementwise, V, Types)
{
        test::valarray::check_compound<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheShiftOperatorsMoveElementsAsABitsetDoes, V, Types)
{
        test::valarray::check_bitset_shifts<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheLogicalOperatorsActElementwise, V, Types)
{
        test::valarray::check_logical<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheComparisonsAnswerAnArrayOfBools, V, Types)
{
        test::valarray::check_comparison<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BeginAndEndMakeItARandomAccessRange, V, Types)
{
        test::valarray::check_range<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(TheBitAlgorithmsReadAndWriteIt, V, Types)
{
        test::valarray::check_algorithms<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BitConvertKeepsPositionsAndASpanViewsIt, V, Types)
{
        test::valarray::check_interop<V>();
}

BOOST_AUTO_TEST_CASE(TheDefaultBlockIsSizeT)
{
        static_assert(std::same_as<xstd::bit_fixed_valarray<3>, xstd::basic_bit_fixed_valarray<std::size_t, 3>>);
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

// At a whole number of blocks every bit is a position, so the array is a structural type, as bit_array is.
BOOST_AUTO_TEST_CASE(AtAWholeNumberOfBlocksItIsStructural)
{
        static_assert(test::structural<xstd::bit_fixed_valarray<64>>);
        static_assert(test::structural<xstd::basic_bit_fixed_valarray<std::uint8_t, 16>>);
        static_assert(not test::structural<xstd::bit_fixed_valarray<3>>);
}

BOOST_AUTO_TEST_SUITE_END()
