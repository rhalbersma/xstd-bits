//          Copyright Rein Halbersma 2014-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <test/valarray.hpp>                  // check_*
#include <xstd/bits/bit_bounded_valarray.hpp> // basic_bit_bounded_valarray, bit_bounded_valarray
#include <boost/test/unit_test.hpp>           // BOOST_AUTO_TEST_CASE, BOOST_AUTO_TEST_CASE_TEMPLATE, BOOST_AUTO_TEST_SUITE, BOOST_AUTO_TEST_SUITE_END, BOOST_CHECK_EQUAL, BOOST_CHECK_THROW
#include <concepts>                           // same_as
#include <cstddef>                            // size_t
#include <cstdint>                            // uint64_t, uint8_t
#include <new>                                // bad_alloc
#include <tuple>                              // tuple

BOOST_AUTO_TEST_SUITE(BitBoundedValarray)

// No capacity, a capacity inside a block and across several, one short of a whole block, and whole blocks.
using Types = std::tuple<
        xstd::basic_bit_bounded_valarray<std::uint8_t, 0>,
        xstd::basic_bit_bounded_valarray<std::uint8_t, 5>,
        xstd::basic_bit_bounded_valarray<std::uint8_t, 17>,
        xstd::basic_bit_bounded_valarray<std::uint64_t, 63>,
        xstd::basic_bit_bounded_valarray<std::uint64_t, 128>,
        xstd::bit_bounded_valarray<130>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(ItIsValarrayShapedAndNoContainer, V, Types)
{
        test::valarray::check_type<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstruction, V, Types)
{
        test::valarray::check_default<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CountConstruction, V, Types)
{
        test::valarray::check_count<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ListConstruction, V, Types)
{
        test::valarray::check_list<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FromBlocks, V, Types)
{
        test::valarray::check_from_blocks<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Assign, V, Types)
{
        test::valarray::check_assign<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Subscript, V, Types)
{
        test::valarray::check_subscript<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SumMinMax, V, Types)
{
        test::valarray::check_reductions<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ShiftCshift, V, Types)
{
        test::valarray::check_shift<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Apply, V, Types)
{
        test::valarray::check_apply<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Resize, V, Types)
{
        test::valarray::check_resize<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Swap, V, Types)
{
        test::valarray::check_swap<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(LogicalNot, V, Types)
{
        test::valarray::check_unary<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CompoundAssign, V, Types)
{
        test::valarray::check_compound<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BitsetShifts, V, Types)
{
        test::valarray::check_bitset_shifts<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(LogicalOperators, V, Types)
{
        test::valarray::check_logical<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Comparisons, V, Types)
{
        test::valarray::check_comparison<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BeginEnd, V, Types)
{
        test::valarray::check_range<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Algorithms, V, Types)
{
        test::valarray::check_algorithms<V>();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(BitConvertAndViews, V, Types)
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

BOOST_AUTO_TEST_SUITE_END()
