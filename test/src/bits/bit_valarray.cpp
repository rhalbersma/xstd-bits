//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/block_types.hpp>       // all_block_types
#include <test/valarray.hpp>          // check_*
#include <xstd/bits/bit_valarray.hpp> // basic_bit_valarray, bit_valarray
#include <boost/test/unit_test.hpp>   // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_THROW
#include <cstddef>                    // size_t
#include <cstdint>                    // uint8_t
#include <limits>                     // numeric_limits
#include <memory>                     // allocator
#include <stdexcept>                  // length_error
#include <tuple>                      // tuple
#include <utility>                    // declval

BOOST_AUTO_TEST_SUITE(BitValarray)

template<class... Blocks>
auto valarrays(std::tuple<Blocks...>) -> std::tuple<xstd::basic_bit_valarray<Blocks>...>;

// Every block the library takes, each over a heap.
using Types = decltype(valarrays(std::declval<test::all_block_types>()));

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
        static_assert(std::same_as<xstd::bit_valarray, xstd::basic_bit_valarray<std::size_t, std::allocator<std::size_t>>>);
}

// A size no difference_type can count is refused as std::vector<bool> refuses it, before anything is allocated.
BOOST_AUTO_TEST_CASE(ASizePastTheAddressableOnesThrowsLengthError)
{
        constexpr auto n = std::numeric_limits<std::size_t>::max();
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_valarray(n)), std::length_error);
        BOOST_CHECK_THROW(static_cast<void>(xstd::bit_valarray(true, n)), std::length_error);
        auto v = xstd::basic_bit_valarray<std::uint8_t>(3UZ);
        BOOST_CHECK_THROW(v.resize(n), std::length_error);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
}

// Constant evaluation allocates and frees within one expression, as std::vector's does.
BOOST_AUTO_TEST_CASE(ItIsUsableInConstantExpressions)
{
        static_assert([] {
                auto v = xstd::bit_valarray{true, false, true};
                auto w = (v.cshift(1) | v) == true;
                return w.min() and v.sum() and not(v < v).max();
        }());
}

BOOST_AUTO_TEST_SUITE_END()
