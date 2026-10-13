//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                          // check_*
#include <xstd/bits/ext/boost/bit_small_valarray.hpp> // basic_bit_small_valarray, bit_small_valarray
#include <boost/container/new_allocator.hpp>          // new_allocator
#include <boost/test/unit_test.hpp>                   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL
#include <concepts>                                   // same_as
#include <cstddef>                                    // size_t
#include <cstdint>                                    // uint64_t, uint8_t
#include <tuple>                                      // tuple

BOOST_AUTO_TEST_SUITE(ExtBoostBitSmallValarray)

// Inline capacity of no block, of part of one, and of two; every size the checks take runs past each onto the heap.
using Types = std::tuple<
        xstd::basic_bit_small_valarray<std::uint8_t, 0>,
        xstd::basic_bit_small_valarray<std::uint8_t, 17>,
        xstd::basic_bit_small_valarray<std::uint64_t, 128>,
        xstd::bit_small_valarray<130>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(TheTypeIsShapedLikeAValarrayAndNotAContainer, V, Types)
{
        test::valarray::check_type<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ADefaultArrayIsEmpty, V, Types)
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

BOOST_AUTO_TEST_CASE_TEMPLATE(APointerConstructsThatManyElements, V, Types)
{
        test::valarray::check_pointer<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ASliceGathersAndItsProxyWritesThrough, V, Types)
{
        test::valarray::check_slice<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AGsliceGathersAndItsProxyWritesThrough, V, Types)
{
        test::valarray::check_gslice<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AMaskGathersAndItsProxyWritesThrough, V, Types)
{
        test::valarray::check_mask<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(AnIndexArrayGathersAndItsProxyWritesThrough, V, Types)
{
        test::valarray::check_indirect<V>();
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

// The short name is the general one at its defaults, the allocator Boost's own as small_vector defaults to.
BOOST_AUTO_TEST_CASE(TheShortNameIsTheGeneralOneAtItsDefaults)
{
        static_assert(std::same_as<xstd::bit_small_valarray<130>, xstd::basic_bit_small_valarray<std::size_t, 130, boost::container::new_allocator<std::size_t>>>);
        BOOST_CHECK(true);
}

// Past the inline capacity the blocks move to the heap, and the elements stay what they were.
BOOST_AUTO_TEST_CASE(GrowingPastTheInlineCapacityKeepsEveryElement)
{
        using V = xstd::basic_bit_small_valarray<std::uint8_t, 17>;
        auto v  = V(true, 17UZ);
        v       = V(true, 200UZ);
        BOOST_CHECK_EQUAL(v.size(), 200UZ);
        BOOST_CHECK(v.min());
        v.resize(3UZ);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
        BOOST_CHECK(not v.max());
}

BOOST_AUTO_TEST_SUITE_END()
