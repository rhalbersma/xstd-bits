//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                   // check_*
#include <xstd/bits/bit_bounded_valarray.hpp>  // basic_bit_bounded_valarray, bit_bounded_valarray
#include <xstd/bits/detail/bounded_blocks.hpp> // IWYU pragma: keep; XSTD_BITS_HAS_CONSTEXPR_BOUNDED
#include <boost/test/unit_test.hpp>            // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <concepts>                            // same_as
#include <cstddef>                             // size_t
#include <cstdint>                             // uint64_t, uint8_t
#include <new>                                 // bad_alloc
#include <tuple>                               // tuple

BOOST_AUTO_TEST_SUITE(BitBoundedValarray)

// No capacity, a capacity inside a block and across several, one short of a whole block, and whole blocks.
using Types = std::tuple<
        xstd::basic_bit_bounded_valarray<std::uint8_t, 0>,
        xstd::basic_bit_bounded_valarray<std::uint8_t, 5>,
        xstd::basic_bit_bounded_valarray<std::uint8_t, 17>,
        xstd::basic_bit_bounded_valarray<std::uint64_t, 63>,
        xstd::basic_bit_bounded_valarray<std::uint64_t, 128>,
        xstd::bit_bounded_valarray<130>>;

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

BOOST_AUTO_TEST_CASE(TheDefaultBlockIsSizeT)
{
        static_assert(std::same_as<xstd::bit_bounded_valarray<3>, xstd::basic_bit_bounded_valarray<std::size_t, 3>>);
}

// Past the capacity is std::inplace_vector's std::bad_alloc, and a refused resize leaves the array as it was.
BOOST_AUTO_TEST_CASE(PastTheCapacityThrowsBadAlloc)
{
        using V = xstd::basic_bit_bounded_valarray<std::uint8_t, 17>;
        BOOST_CHECK_THROW(static_cast<void>(V(18UZ)), std::bad_alloc);
        BOOST_CHECK_THROW(static_cast<void>(V(true, 18UZ)), std::bad_alloc);
        BOOST_CHECK_THROW(static_cast<void>(xstd::basic_bit_bounded_valarray<std::uint8_t, 2>({true, false, true})), std::bad_alloc);
        auto v = V(true, 3UZ);
        BOOST_CHECK_THROW(v.resize(18UZ), std::bad_alloc);
        BOOST_CHECK_EQUAL(v.size(), 3UZ);
        BOOST_CHECK(v.min());
}

#ifdef XSTD_BITS_HAS_CONSTEXPR_BOUNDED

// Over std::inplace_vector the blocks are the object, so constant evaluation needs no allocation.
BOOST_AUTO_TEST_CASE(ItIsUsableInConstantExpressions)
{
        static_assert([] -> bool {
                auto v = xstd::bit_bounded_valarray<70>{true, false, true};
                v.resize(68UZ, true);
                return (v >> 67UZ)[0] and not(v << 68UZ).max();
        }());
}

// A mask gathers, its proxy writes, and an array is read from a pointer and from a proxy, in a constant expression.
BOOST_AUTO_TEST_CASE(TheMaskSubscriptIsUsableInConstantExpressions)
{
        static_assert([] -> bool {
                using V   = xstd::bit_bounded_valarray<70>;
                auto mask = V(false, 68UZ);
                mask[1]   = true;
                mask[65]  = true;
                auto v    = V(false, 68UZ);
                v[mask]   = V{true, true};
                v[mask] &= V{false, true};
                bool const bools[3] = {true, false, true}; // NOLINT(modernize-avoid-c-arrays): what the pointer reads.
                auto const& read    = v;
                auto const gathered = read[mask];
                auto const proxied  = V(v[mask]);
                return not gathered[0] and gathered[1] and (gathered == proxied).min() and V(bools, 3UZ)[2];
        }());
}

#endif

BOOST_AUTO_TEST_SUITE_END()
